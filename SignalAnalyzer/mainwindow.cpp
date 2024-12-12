#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Настройка интерфейса
    ui_settings();
    setup_graph();

    // Сигналы и слоты
    connect(ui->oscilloscope, &QRadioButton::clicked, this, &MainWindow::setup_graph);
    connect(ui->spectrum_analyzer, &QRadioButton::clicked, this, &MainWindow::setup_graph);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ui_settings()
{
    // Режим осциллографа по умолчанию
    ui->oscilloscope->setChecked(true);
    // Включены каналы по умолчанию
    ui->channel_1->setChecked(true);
    ui->channel_2->setChecked(true);
}

void MainWindow::setup_graph()
{
    // Ускоритель OpenGL
    ui->graph->setOpenGl(true);
    // Масштабирование
    ui->graph->setInteraction(QCP::iRangeZoom, true);
    ui->graph->setInteraction(QCP::iRangeDrag, true);
    // Цвета графика
    ui->graph->setBackground(Qt::black);
    ui->graph->xAxis->setTickLabelColor(Qt::white);
    ui->graph->xAxis->setLabelColor(Qt::white);
    ui->graph->xAxis->setBasePen(QPen(Qt::white));
    ui->graph->xAxis->setTickPen(QPen(Qt::white));
    ui->graph->yAxis->setTickLabelColor(Qt::white);
    ui->graph->yAxis->setLabelColor(Qt::white);
    ui->graph->yAxis->setBasePen(QPen(Qt::white));
    ui->graph->yAxis->setTickPen(QPen(Qt::white));

    // Настройки осей
    if (ui->oscilloscope->isChecked())
    {
        ui->graph->xAxis->setLabel("Время, с");
        ui->graph->yAxis->setLabel("Напряжение, В");
    }
    else if (ui->spectrum_analyzer->isChecked())
    {
        ui->graph->xAxis->setLabel("Частота, Гц");
        ui->graph->yAxis->setLabel("Уровень сигнала, дБм");
    }

    ui->graph->clearGraphs();
    ui->graph->replot();
}

void MainWindow::on_file_path_button_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Открыть CSV файл с данными"), "/home/", tr("CSV data (*.csv)"));
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Ошибка", "Невозможно открыть файл: " + file.errorString());
        return;
    }

    // Настройка таблицы
    ui->data_table->clear();
    ui->data_table->setRowCount(0);
    ui->data_table->setColumnCount(3);
    QStringList headers = {"Time (s)", "CH1 (V)", "CH2 (V)"};
    ui->data_table->setHorizontalHeaderLabels(headers);

    QTextStream in(&file);
    int lineNumber = 0;
    double sampleRate = 0;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        lineNumber++;
        if (lineNumber == 1) // Чтение заголовка для tInc
        {
            QStringList headerParts = line.split(',');
            for (const QString &part : headerParts)
            {
                if (part.trimmed().startsWith("tInc"))
                {
                    QString tIncValue = part.split('=').last().trimmed();
                    bool ok;
                    double tInc = tIncValue.toDouble(&ok);
                    if (ok && tInc > 0)
                    {
                        sampleRate = 1.0 / tInc;
                        ui->sample_rate_edit->setText(QString::number(sampleRate, 'f', 2) + " Hz");
                    }
                }
            }
            continue;
        }

        QStringList values = line.split(',');
        if (values.size() < 3)
        {
            qWarning() << "Неверный формат данных в строке:" << line;
            continue;
        }

        bool ok1, ok2, ok3;
        double time = values[0].toDouble(&ok1);
        double ch1 = values[1].toDouble(&ok2);
        double ch2 = values[2].toDouble(&ok3);

        if (ok1 && ok2 && ok3)
        {
            int row = ui->data_table->rowCount();
            ui->data_table->insertRow(row);
            ui->data_table->setItem(row, 0, new QTableWidgetItem(QString::number(time)));
            ui->data_table->setItem(row, 1, new QTableWidgetItem(QString::number(ch1)));
            ui->data_table->setItem(row, 2, new QTableWidgetItem(QString::number(ch2)));
        }
        else
        {
            qWarning() << "Ошибка преобразования данных в строке:" << line;
        }
    }

    file.close();
    QMessageBox::information(this, "Успех", "Файл успешно прочитан!");
    ui->file_path_edit->setText(filePath);
}

void MainWindow::on_plot_graph_button_clicked()
{
    ui->graph->clearGraphs();
    int rowCount = ui->data_table->rowCount();
    if (rowCount == 0)
        return;

    QVector<double> time, ch1, ch2;

    for (int row = 0; row < rowCount; ++row)
    {
        time.append(ui->data_table->item(row, 0)->text().toDouble());
        ch1.append(ui->data_table->item(row, 1)->text().toDouble());
        ch2.append(ui->data_table->item(row, 2)->text().toDouble());
    }

    if (ui->channel_1->isChecked())
    {
        ui->graph->addGraph();
        ui->graph->graph(0)->setData(time, ch1);
        ui->graph->graph(0)->setPen(QPen(QColor(Qt::cyan), 2));
        ui->graph->graph(0)->setName("Канал 1");
    }

    if (ui->channel_2->isChecked())
    {
        int index = ui->channel_1->isChecked() ? 1 : 0;
        ui->graph->addGraph();
        ui->graph->graph(index)->setData(time, ch2);
        ui->graph->graph(index)->setPen(QPen(QColor(Qt::yellow), 2));
        ui->graph->graph(index)->setName("Канал 2");
    }

    ui->graph->rescaleAxes();
    ui->graph->replot();

    // Обновление масштабов
    update_scale_edits();
}

void MainWindow::update_scale_edits()
{
    double xRange = ui->graph->xAxis->range().size();
    double yRange = ui->graph->yAxis->range().size();

    ui->scale_x_edit->setText(QString::number(xRange * 1e6, 'f', 2) + " мкс");
    ui->scale_y_edit->setText(QString::number(yRange * 1e3, 'f', 2) + " мВ");
}

void MainWindow::on_auto_scale_button_clicked()
{
    ui->graph->rescaleAxes();
    ui->graph->replot();
    update_scale_edits();
}

void MainWindow::on_data_export_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Сохранить CSV"), "/home/", tr("CSV Files (*.csv)"));
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "Ошибка", "Невозможно сохранить файл: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    QStringList headers = {"Time (s)", "CH1 (V)", "CH2 (V)"};
    out << headers.join(',') << "\n";

    for (int row = 0; row < ui->data_table->rowCount(); ++row)
    {
        QStringList rowData;
        for (int col = 0; col < ui->data_table->columnCount(); ++col)
        {
            rowData << ui->data_table->item(row, col)->text();
        }
        out << rowData.join(',') << "\n";
    }

    file.close();
    QMessageBox::information(this, "Успех", "Данные успешно экспортированы!");
}

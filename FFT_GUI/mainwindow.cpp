#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setup_graph_settings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_graph_settings()
{
    // Ускоритель OpenGL
    ui->oscilloscope_graph->setOpenGl(true);
    ui->oscilloscope_graph->setOpenGl(true);

    // Масштабирование
    ui->oscilloscope_graph->setInteraction(QCP::iRangeZoom, true);
    ui->oscilloscope_graph->setInteraction(QCP::iRangeDrag, true);

    ui->spectrum_graph->setInteraction(QCP::iRangeZoom, true);
    ui->spectrum_graph->setInteraction(QCP::iRangeDrag, true);

    // Цвета графиков
    ui->oscilloscope_graph->setBackground(Qt::black);
    ui->oscilloscope_graph->xAxis->setTickLabelColor(Qt::white);
    ui->oscilloscope_graph->xAxis->setLabelColor(Qt::white);
    ui->oscilloscope_graph->xAxis->setBasePen(QPen(Qt::white));
    ui->oscilloscope_graph->xAxis->setTickPen(QPen(Qt::white));
    ui->oscilloscope_graph->yAxis->setTickLabelColor(Qt::white);
    ui->oscilloscope_graph->yAxis->setLabelColor(Qt::white);
    ui->oscilloscope_graph->yAxis->setBasePen(QPen(Qt::white));
    ui->oscilloscope_graph->yAxis->setTickPen(QPen(Qt::white));

    ui->spectrum_graph->setBackground(Qt::black);
    ui->spectrum_graph->xAxis->setTickLabelColor(Qt::white);
    ui->spectrum_graph->xAxis->setLabelColor(Qt::white);
    ui->spectrum_graph->xAxis->setBasePen(QPen(Qt::white));
    ui->spectrum_graph->xAxis->setTickPen(QPen(Qt::white));
    ui->spectrum_graph->yAxis->setTickLabelColor(Qt::white);
    ui->spectrum_graph->yAxis->setLabelColor(Qt::white);
    ui->spectrum_graph->yAxis->setBasePen(QPen(Qt::white));
    ui->spectrum_graph->yAxis->setTickPen(QPen(Qt::white));

    // Настройки осей
    ui->oscilloscope_graph->xAxis->setLabel("Время, с");
    ui->oscilloscope_graph->yAxis->setLabel("Напряжение, В");
    ui->spectrum_graph->xAxis->setLabel("Частота, Гц");
    ui->spectrum_graph->yAxis->setLabel("Уровень сигнала, дБм");

    // Очистка графиков
    ui->oscilloscope_graph->clearGraphs();
    ui->spectrum_graph->clearGraphs();

    // Перестроить график
    ui->oscilloscope_graph->replot();
    ui->spectrum_graph->replot();

}

void MainWindow::on_choose_file_button_clicked()
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

    // Считывание файла целиком
    QByteArray fileData = file.readAll();
    file.close();

    // Обработка данных
    QVector<double> t, u, f, a;
    QStringList lines = QString(fileData).split('\n', Qt::SkipEmptyParts);

    for (int i = 1; i < lines.size(); ++i) // Пропускаем заголовок
    {
        QStringList values = lines[i].split(',');
        if (values.size() < 4)
        {
            qWarning() << "Неверный формат данных в строке:" << lines[i];
            continue;
        }

        bool ok1, ok2, ok3, ok4;
        double time = values[0].toDouble(&ok1);
        double voltage = values[1].toDouble(&ok2);
        double frequency = values[2].toDouble(&ok3);
        double amplitude = values[3].toDouble(&ok4);

        if (ok1 && ok2 && ok3 && ok4)
        {
            t.append(time);
            u.append(voltage);
            f.append(frequency);
            a.append(amplitude);
        }
        else
        {
            qWarning() << "Ошибка преобразования данных в строке:" << lines[i];
        }
    }

    QMessageBox::information(this, "Успех", "Файл успешно прочитан!");
    ui->file_path_edit->setText(filePath);

    // Построение графиков
    ui->oscilloscope_graph->addGraph();
    ui->oscilloscope_graph->graph(0)->setData(t, u);
    ui->oscilloscope_graph->graph(0)->setPen(QPen(QColor(Qt::cyan), 2));
    ui->oscilloscope_graph->graph(0)->setName("Осциллограмма сигнала");

    ui->spectrum_graph->addGraph();
    ui->spectrum_graph->graph(0)->setData(f, a);
    ui->spectrum_graph->graph(0)->setPen(QPen(QColor(Qt::yellow), 2));
    ui->spectrum_graph->graph(0)->setName("АЧХ сигнала");

    ui->oscilloscope_graph->rescaleAxes();
    ui->spectrum_graph->rescaleAxes();

    ui->oscilloscope_graph->replot();
    ui->spectrum_graph->replot();
}





void MainWindow::on_autoscale_button_clicked()
{
    ui->oscilloscope_graph->rescaleAxes();
    ui->spectrum_graph->rescaleAxes();

    ui->oscilloscope_graph->replot();
    ui->spectrum_graph->replot();
}


void MainWindow::on_clear_button_clicked()
{
    ui->oscilloscope_graph->clearGraphs();
    ui->spectrum_graph->clearGraphs();
    on_autoscale_button_clicked();
    ui->file_path_edit->setText("");
}


void MainWindow::on_about_prog_triggered()
{
    form.show();
}


void MainWindow::on_save_oscillogram_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Сохранить график как PNG"), "/home/", tr("Изображения (*.png)"));
    if (!filePath.isEmpty())
        ui->oscilloscope_graph->savePng(filePath);
}


void MainWindow::on_save_spectrum_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Сохранить график как PNG"), "/home/", tr("Изображения (*.png)"));
    if (!filePath.isEmpty())
        ui->spectrum_graph->savePng(filePath);
}


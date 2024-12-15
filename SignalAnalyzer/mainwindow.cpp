#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QtMath>

#include <math.h>
#include <mpi.h>
#include <omp.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Настройка интерфейса
    ui_settings();
    setup_graph();


    // Генерируем тестовую выборку
    sample_signal(&MainWindow::my_signal, 256, &u, &t);

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



/// Искусственный сигнал для проверки работоспособности алгоритма
void MainWindow::sample_signal(double (MainWindow::*f)(double), int m, QVector<double> *x, QVector<double> *y) {
    if (m <= 0 || x == nullptr || y == nullptr) {
        return;
    }

    x->clear();
    y->clear();

    double dt = 1.0 / m;

    for (int i = 0; i < m; ++i) {
        double t = i * dt;
        x->append(t);
        y->append((this->*f)(t)); // Вызов метода через указатель на функцию-член
    }
}

double MainWindow::my_signal(double t)
{
    return 3 * cos(2 * M_PI * 3 * t + M_PI / 4)   // Гармоника с частотой 3 Гц, амплитудой 3, сдвиг фазы π/4
           + 2 * sin(2 * M_PI * 7 * t - M_PI / 6)  // Гармоника с частотой 7 Гц, амплитудой 2, сдвиг фазы -π/6
           + 1.5 * cos(2 * M_PI * 12 * t)          // Гармоника с частотой 12 Гц, амплитудой 1.5, без фазового сдвига
           + 0.8 * sin(2 * M_PI * 20 * t + M_PI / 3); // Гармоника с частотой 20 Гц, амплитудой 0.8, сдвиг фазы π/3
}

void MainWindow::compute_fft(const QVector<double> &t, const QVector<double> &u, QVector<double> &frequencies, QVector<double> &amplitudes)
{
    int rank, size;

    // Инициализация MPI
    MPI_Init(nullptr, nullptr); // MPI_Init должен быть вызван до работы с MPI

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = t.size();
    if (N != u.size() || N == 0) {
        MPI_Finalize(); // Завершаем MPI в случае ошибки
        return;
    }

    // Ваш существующий код
    frequencies.clear();
    amplitudes.clear();

    int local_N = N / size;
    int start = rank * local_N;
    int end = (rank == size - 1) ? N : start + local_N;

    QVector<std::complex<double>> local_fft(local_N);

#pragma omp parallel for
    for (int k = start; k < end; ++k) {
        std::complex<double> sum(0.0, 0.0);
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            std::complex<double> term(std::cos(angle), std::sin(angle));
            sum += std::complex<double>(u[n], 0.0) * term;
        }
        local_fft[k - start] = sum;
    }

    QVector<std::complex<double>> global_fft;
    if (rank == 0) {
        global_fft.resize(N);
    }

    MPI_Gather(local_fft.data(), local_N * sizeof(std::complex<double>), MPI_BYTE,
               global_fft.data(), local_N * sizeof(std::complex<double>), MPI_BYTE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        double dt = t[1] - t[0];
        double freq_step = 1.0 / (N * dt);

        for (int k = 0; k < N; ++k) {
            frequencies.append(k * freq_step);
            amplitudes.append(std::abs(global_fft[k]) / N);
        }
    }

    MPI_Finalize(); // Завершаем работу MPI
}
///
///
///
///
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

    if (ui->oscilloscope->isChecked())
    {
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
    }
    if (ui->spectrum_analyzer->isChecked())
    {
        compute_fft(time,ch1,f,a);
        if (ui->channel_1->isChecked())
        {
            ui->graph->addGraph();
            ui->graph->graph(0)->setData(f, a);
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
    }

    // compute_fft(t,u,f,a);

    // ui->graph->addGraph();
    // ui->graph->graph(0)->setData(t, u);
    // ui->graph->graph(0)->setPen(QPen(QColor(Qt::cyan), 2));

    // ui->graph->addGraph();
    // ui->graph->graph(1)->setData(f, a);
    // ui->graph->graph(1)->setPen(QPen(QColor(Qt::yellow), 2));


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

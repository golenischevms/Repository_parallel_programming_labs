#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <complex>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_file_path_button_clicked();
    void on_plot_graph_button_clicked();
    void on_data_export_triggered();
    void on_auto_scale_button_clicked();
    void update_scale_edits();


private:
    Ui::MainWindow *ui;

    QVector <double> t, u;
    QVector <double> f, a, f1, f2, a1, a2;

    void ui_settings();
    void setup_graph();



public:
    void sample_signal(double (MainWindow::*f)(double), int m, QVector<double> *x, QVector<double> *y);
    double my_signal(double t); // Объявление функции-члена
    void compute_fft(const QVector<double> &t, const QVector<double> &u, QVector<double> &frequencies, QVector<double> &amplitudes);




};
#endif // MAINWINDOW_H

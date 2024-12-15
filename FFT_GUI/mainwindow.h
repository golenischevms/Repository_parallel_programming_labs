#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <prog_info.h>

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

private:
    Ui::MainWindow *ui;
    prog_info form;

private slots:
    void setup_graph_settings();
    void on_choose_file_button_clicked();
    void on_autoscale_button_clicked();
    void on_clear_button_clicked();
    void on_about_prog_triggered();
    void on_save_oscillogram_triggered();
    void on_save_spectrum_triggered();
};
#endif // MAINWINDOW_H

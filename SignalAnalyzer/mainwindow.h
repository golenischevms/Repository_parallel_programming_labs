#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

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

    void ui_settings();
    void setup_graph();

};
#endif // MAINWINDOW_H

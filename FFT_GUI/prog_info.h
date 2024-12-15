#ifndef PROG_INFO_H
#define PROG_INFO_H

#include <QWidget>

namespace Ui {
class prog_info;
}

class prog_info : public QWidget
{
    Q_OBJECT

public:
    explicit prog_info(QWidget *parent = nullptr);
    ~prog_info();

private:
    Ui::prog_info *ui;
};

#endif // PROG_INFO_H

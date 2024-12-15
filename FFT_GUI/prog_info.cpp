#include "prog_info.h"
#include "ui_prog_info.h"

prog_info::prog_info(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::prog_info)
{
    ui->setupUi(this);
}

prog_info::~prog_info()
{
    delete ui;
}

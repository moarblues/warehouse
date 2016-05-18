#include "dlg_selectrecord.h"
#include "ui_dlg_selectrecord.h"

dlg_selectRecord::dlg_selectRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_selectRecord)
{
    ui->setupUi(this);
}

dlg_selectRecord::~dlg_selectRecord()
{
    delete ui;
}

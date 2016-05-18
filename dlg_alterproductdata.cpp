#include "dlg_alterproductdata.h"
#include "ui_dlg_alterproductdata.h"

dlg_alterProductData::dlg_alterProductData(QWidget *parent) : QDialog(parent), ui(new Ui::dlg_alterProductData)
{
    ui->setupUi(this);
    ui->le_barcode->setPlaceholderText("Штрихкод");
}

dlg_alterProductData::~dlg_alterProductData()
{
    delete ui;
}

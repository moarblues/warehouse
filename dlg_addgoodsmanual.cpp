#include "dlg_addgoodsmanual.h"
#include "ui_dlg_addgoodsmanual.h"

dlg_addGoodsManual::dlg_addGoodsManual(QWidget *parent) : QDialog(parent), ui(new Ui::dlg_addGoodsManual)
{
    ui->setupUi(this);
    this->ui->le_barcode->setPlaceholderText("Штрихкод");
}

dlg_addGoodsManual::~dlg_addGoodsManual()
{
    delete ui;
}

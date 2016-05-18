#include "dlg_addproduct.h"
#include "ui_dlg_addproduct.h"

dlg_addProduct::dlg_addProduct(QWidget *parent) : QDialog(parent), ui(new Ui::dlg_addProduct)
{
    ui->setupUi(this);
}

dlg_addProduct::~dlg_addProduct()
{
    delete ui;
}

void dlg_addProduct::on_btn_search_clicked()
{
    emit this->sendRequest(QStringList() << "dap" << "db" << "request" << "0"
                         << QString("select,lists,products,").append(this->ui->le_name->text()).append("::::"));
}

void dlg_addProduct::handleResult(const QString &sTarget, const QVector<QStringList> &vslResult)
{
//    QStringList slTarget = sTarget.split("::");

    if(QString::compare(sTarget,"productlist")==0) {this->fillResultTable(vslResult);}
//    else if(QString::compare(slTarget.at(2),"dbinfo")==0) {this->fillCbDepots(vslResult);}
}

void dlg_addProduct::fillResultTable(const QVector<QStringList> &vslResult)
{
//    qDebug() << vslResult;
    if(this->ui->tvw_selectData->model()!=0) {delete this->ui->tvw_selectData->model();}
    QStandardItemModel *modelIn= new QStandardItemModel(this);

    for (int irow=0; irow<vslResult.size(); ++irow)
    {
        if (vslResult.at(irow).size()!=9)
        {this->logger(true,"Ошибка структуры результата");break;}

        QStandardItem *siRecordId = new QStandardItem(vslResult.at(irow).at(0));
        QStandardItem *siRegistryRef = new QStandardItem(vslResult.at(irow).at(1));
        QStandardItem *siProductTypeAlias = new QStandardItem(vslResult.at(irow).at(2));
        QStandardItem *siProductName = new QStandardItem(vslResult.at(irow).at(3));
        QStandardItem *siProductManufactorerName = new QStandardItem(vslResult.at(irow).at(4));
        QStandardItem *siProductCountryName = new QStandardItem(vslResult.at(irow).at(5));
        QStandardItem *siProductUnitName = new QStandardItem(vslResult.at(irow).at(6));
        QStandardItem *siSellPrice = new QStandardItem(vslResult.at(irow).at(7));
        QStandardItem *siBarcode = new QStandardItem(vslResult.at(irow).at(8));

        QList<QStandardItem *> lsiModelRow;
        lsiModelRow.append(siRecordId);
        lsiModelRow.append(siRegistryRef);
        lsiModelRow.append(siProductTypeAlias);
        lsiModelRow.append(siProductName);
        lsiModelRow.append(siProductManufactorerName);
        lsiModelRow.append(siProductCountryName);
        lsiModelRow.append(siProductUnitName);
        lsiModelRow.append(siSellPrice);
        lsiModelRow.append(siBarcode);
        modelIn->appendRow(lsiModelRow);
    }
    modelIn->setHorizontalHeaderLabels(QStringList() << "№" << "Ссылка" << "Тип" << "Наименование"
                                           << "Производитель" << "Страна" << "Ед. изм." << "Цена" << "Штрихкод");

    ui->tvw_selectData->setModel(modelIn);
    int effWidth = ui->tvw_selectData->width();
    ui->tvw_selectData->setColumnWidth(0, effWidth * 0.05);
    ui->tvw_selectData->setColumnWidth(1, effWidth * 0.05);
    ui->tvw_selectData->setColumnWidth(2, effWidth * 0.05);
    ui->tvw_selectData->setColumnWidth(3, effWidth * 0.25);
    ui->tvw_selectData->setColumnWidth(4, effWidth * 0.15);
    ui->tvw_selectData->setColumnWidth(5, effWidth * 0.15);
    ui->tvw_selectData->setColumnWidth(6, effWidth * 0.05);
    ui->tvw_selectData->setColumnWidth(7, effWidth * 0.05);
    ui->tvw_selectData->setColumnWidth(8, effWidth * 0.1);
}

void dlg_addProduct::logger(bool stType, QString sLine)
{
    if (stType==true)
    {QMessageBox::critical(this,"Ошибка",sLine,QMessageBox::Ok);}
}

void dlg_addProduct::on_btn_add_clicked()
{

}

void dlg_addProduct::on_btn_cancel_clicked()
{
    this->close();
}

#ifndef DLG_ADDPRODUCT_H
#define DLG_ADDPRODUCT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class dlg_addProduct;
}

class dlg_addProduct : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_addProduct(QWidget *parent = 0);
    ~dlg_addProduct();

signals:
    void sendRequest(const QStringList &slRqData);

private slots:
    void on_btn_search_clicked();

    void on_btn_add_clicked();

    void on_btn_cancel_clicked();

public slots:
    void handleResult(const QString &sTarget, const QVector<QStringList> &vslResult);
    void fillResultTable(const QVector<QStringList> &vslResult);
    void logger(bool stType, QString sLine);

private:
    Ui::dlg_addProduct *ui;
};

#endif // DLG_ADDPRODUCT_H

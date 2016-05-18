#ifndef DLG_ALTERPRODUCTDATA_H
#define DLG_ALTERPRODUCTDATA_H

#include <QDialog>

namespace Ui {
class dlg_alterProductData;
}

class dlg_alterProductData : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_alterProductData(QWidget *parent = 0);
    ~dlg_alterProductData();

private:
    Ui::dlg_alterProductData *ui;
};

#endif // DLG_ALTERPRODUCTDATA_H

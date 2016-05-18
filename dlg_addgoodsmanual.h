#ifndef DLG_ADDGOODSMANUAL_H
#define DLG_ADDGOODSMANUAL_H

#include <QDialog>

namespace Ui {
class dlg_addGoodsManual;
}

class dlg_addGoodsManual : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_addGoodsManual(QWidget *parent = 0);
    ~dlg_addGoodsManual();

private:
    Ui::dlg_addGoodsManual *ui;
};

#endif // DLG_ADDGOODSMANUAL_H

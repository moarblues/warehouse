#ifndef DLG_SELECTRECORD_H
#define DLG_SELECTRECORD_H

#include <QDialog>

namespace Ui {
class dlg_selectRecord;
}

class dlg_selectRecord : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_selectRecord(QWidget *parent = 0);
    ~dlg_selectRecord();

private:
    Ui::dlg_selectRecord *ui;
};

#endif // DLG_SELECTRECORD_H

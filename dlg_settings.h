#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QUrl>
#include <QPointer>
#include <QDebug>

#include "files.h"

namespace Ui {
class dlg_settings;
}

class dlg_settings : public QDialog
{
    Q_OBJECT

    QStringList slSettings;
    QList<QStringList> lslLibInfo;
    QStandardItemModel *modelProcLibs;

    void loadSettings();
    void saveSettings();
    
public:
    explicit dlg_settings(QWidget *parent = 0);
    ~dlg_settings();

public slots:
    void logger(const bool &stType,const QString &sError);
    
private slots:
    void on_btn_invOpen_clicked();
    void on_btn_invChange_clicked();
    void on_btn_plOpen_clicked();
    void on_btn_plChange_clicked();
    void on_btn_add_clicked();
    void on_btn_delete_clicked();
    void on_btn_refresh_clicked();

private:
    Ui::dlg_settings *ui;
};

#endif // DLG_SETTINGS_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QCloseEvent>
#include <QDebug>

#include "files.h"
#include "client.h"
#include "dlg_settings.h"
#include "dlg_addproduct.h"
#include "dlg_alterproductdata.h"

namespace Ui {
    class mainWindow;
}

class mainWindow : public QMainWindow
{
    Q_OBJECT

    QLabel *lblSrvStatus;
    QLabel *lblClientStatus;
    QLabel *lblInfo;
    QProgressBar *pbProgress;

    QTimer *timerConnect;
    QTimer *timerCheck;
    QStringList slSettings;
    QString sClientID;
    sslClient *scw;
    QThread *tSslClient;
    QList<QStringList> lslLibInfo;
    QVector<QStringList> vslTable;
    unsigned short serverStatus;
    unsigned short clientStatus;
    bool socketIsOperating;
    QList<QStringList> lslRequestStamp;
    
public:
    explicit mainWindow(QWidget *parent = 0);
    ~mainWindow();

signals:
    void connectClient(const QString &sReqStamp, const QString &sServerIp);
    void disconnectClient();
    void processClient(const QString &sReqStamp, const QByteArray &baData);
    void sendResult(const QString &sTarget, const QVector<QStringList> &vslResult);
    void bufferAccepted();
    
private slots:
    void lockCurrentTab();
    void unlockCurrentTab();
    void getServerStatus();
    void on_btn_processIn_clicked();
    void on_btn_addOut_clicked();
    void on_btn_deleteOut_clicked();
    void on_btn_processOut_clicked();
    void on_cb_vendors_activated(int index);
    void on_act_exit_triggered();
    void on_act_settings_triggered();
    void on_btn_getDepotList_clicked();
    void on_cb_depots_currentTextChanged(const QString &arg1);
    void on_btn_addRecord_clicked();
    void on_tvw_inProc_doubleClicked(const QModelIndex &index);
    void on_btn_getRegistryTypes_clicked();
    void on_btn_selectRegistry_clicked();
    void on_btn_registerProduct_clicked();
    void on_btn_alterProductData_clicked();
    void on_btn_selectRetail_clicked();

public slots:
    void closeEvent(QCloseEvent *event);
    void loadSettings();
    void loadProcLibs();
    void setupAdditionalElements();
    void createSocket();
    void removeSocket();
    void connectSocket(const QByteArray &baSrvIp);
    void authenticateClient();
    void getServerIp();
    void setSocketProgressExpected(qint64 expected);
    void setSocketProgressCurrent(qint64 current);
    void resultFilter(const QStringList &slTarget, const QVector<QStringList> &vslResult);
    void processServerLocalRespond(const QStringList &slTarget, const QVector<QStringList> &vslResult);
    void processServerRemoteRespond(const QStringList &slTarget, const QVector<QStringList> &vslResult);
    void processDbRespond(const QString &sTarget, const QVector<QStringList> &vslResult);
    void fillTableInvoice();
    void enqueueRequest(const QStringList &slRequestData);
    void executeRequest();
    QString completeRequest(const QString &sReqStamp);
    void sendData(const QString &sReqStamp, const QString &sSendData);
    void setServerStatus(const QString &sStatus);
    void setClientStatus(const QString &sStatus);
    void fillRequestQueueModel();
    void fillCbDepots(const QVector<QStringList> &vslResult);
    void fillCbRegTypes(const QVector<QStringList> &vslResult);
    void fillRetailModel(const QVector<QStringList> &vslResult);
    void fillRegistryModel(const QVector<QStringList> &vslResult);
    void logger(bool stType, QString sLine);

private:
    Ui::mainWindow *ui;
};

#endif // MAINWINDOW_H

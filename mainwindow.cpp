#include "mainwindow.h"
#include "ui_mainwindow.h"

mainWindow::mainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType <QVector<QStringList> >("QVector<QStringList>");
    serverStatus = 0;
    clientStatus = 0;
    socketIsOperating = false;

    this->setupAdditionalElements();

    QDesktopWidget *desktop = QApplication::desktop();
    this->move((desktop->width()-this->width())/2,((desktop->height()-this->height()))/2);
    this->resize(desktop->availableGeometry().width()-20,desktop->availableGeometry().height());

    timerConnect = new QTimer(this);
    connect(timerConnect,SIGNAL(timeout()),this,SLOT(getServerIp()));
    timerConnect->setInterval(30000);
    timerCheck = new QTimer(this);
    connect(timerCheck,SIGNAL(timeout()),this,SLOT(getServerStatus()));
    timerCheck->setInterval(30000);

    this->loadSettings();
    this->loadProcLibs();
    this->createSocket();
}

mainWindow::~mainWindow()
{
    delete lblSrvStatus;
    delete lblClientStatus;
    delete lblInfo;
    delete pbProgress;
    delete ui;
}

void mainWindow::closeEvent(QCloseEvent *event)
{
    this->logger(false, "Ожидание завершения работы");
    this->removeSocket();
    connect(tSslClient,SIGNAL(destroyed()),this,SLOT(close()),Qt::UniqueConnection);

    if (tSslClient->isRunning())
    {event->ignore();return;}
    event->accept();
}

void mainWindow::setupAdditionalElements()
{
    lblSrvStatus = new QLabel;
    lblClientStatus = new QLabel;
    lblInfo = new QLabel;
    pbProgress = new QProgressBar;
    lblSrvStatus->setFixedHeight(20);
    lblSrvStatus->setFixedWidth(30);
    lblClientStatus->setFixedHeight(20);
    lblClientStatus->setFixedWidth(30);
    lblInfo->setText("infolabel");
    lblSrvStatus->setText("S");
    lblSrvStatus->setAlignment(Qt::AlignCenter);
    lblClientStatus->setText("C");
    lblClientStatus->setAlignment(Qt::AlignCenter);
    lblSrvStatus->setAutoFillBackground(true);
    lblClientStatus->setAutoFillBackground(true);
    this->ui->statusBar->addPermanentWidget(lblInfo,4);
    this->ui->statusBar->addPermanentWidget(pbProgress,2);
    this->ui->statusBar->addPermanentWidget(lblSrvStatus,2);
    this->ui->statusBar->addPermanentWidget(lblClientStatus,2);

//    lblSrvStatus->setStyleSheet("QLabel { background-color: red }");
}

void mainWindow::loadSettings()
{
    QPointer<files> fw = new files(this);
    connect(fw, SIGNAL(setStatus(bool,QString)), this, SLOT(logger(bool,QString)));
    slSettings = fw->readSettings();
    delete fw;
}

void mainWindow::loadProcLibs()
{
    ui->cb_vendors->clear();
    QPointer<files> fw = new files(this);
    connect(fw, SIGNAL(setStatus(bool,QString)), this, SLOT(logger(bool,QString)));
    lslLibInfo = fw->loadProcLibs(slSettings.at(1));
    delete fw;

    if (lslLibInfo.isEmpty())
    {this->logger(1, "Директория библиотек-обработчиков пуста"); return;}

    for (int iitm = 0; iitm < lslLibInfo.size(); ++iitm)
    {
        ui->cb_vendors->addItem(lslLibInfo.at(iitm).at(0));
    }
}

void mainWindow::createSocket()
{
    tSslClient = new QThread(this);
    scw = new sslClient();
    scw->moveToThread(tSslClient);

    connect(scw,SIGNAL(setStatus(bool,QString)),this,SLOT(logger(bool,QString)));
    connect(scw,SIGNAL(socketProgressExpected(qint64)),this,SLOT(setSocketProgressExpected(qint64)));
    connect(scw,SIGNAL(socketProgressCurrent(qint64)),this,SLOT(setSocketProgressCurrent(qint64)));
//    connect(sslCl,SIGNAL(processFinished()),this,SLOT(unlockCurrentTab()));
    connect(this,SIGNAL(connectClient(QString,QString)),scw,SLOT(connectToServer(QString,QString)));
    connect(this,SIGNAL(disconnectClient()),scw,SLOT(disconnectFromServer()));
    connect(this,SIGNAL(processClient(QString,QByteArray)),scw,SLOT(processData(QString,QByteArray)));
    connect(scw,SIGNAL(setResult(QStringList,QVector<QStringList>))
            ,this,SLOT(resultFilter(QStringList,QVector<QStringList>)));
    connect(this,SIGNAL(bufferAccepted()),scw,SLOT(clearBuffer()));
    connect(tSslClient,SIGNAL(started()),this,SLOT(getServerIp()));
    connect(scw,SIGNAL(finished()),tSslClient,SLOT(quit()));
    connect(tSslClient,SIGNAL(finished()),scw,SLOT(deleteLater()));
    connect(scw,SIGNAL(destroyed()),tSslClient,SLOT(deleteLater()));
    tSslClient->start();
}

void mainWindow::removeSocket()
{
    if (serverStatus==2) {timerConnect->stop(); serverStatus=0;}
    if (serverStatus==0) {tSslClient->quit();return;}
    this->enqueueRequest(QStringList() << "mw" << "srv" << "request" << "0" << "client-close");
}

void mainWindow::connectSocket(const QByteArray &baSrvIp)
{
    QString sIp(baSrvIp);
//    baSrvIp.clear();
    files *fw = new files(this);
    fw->writeServerIp(sIp);
    delete fw;

//    emit this->connectClient(si);
    emit this->connectClient("local","127.0.0.1");
}

void mainWindow::authenticateClient()
{
    this->enqueueRequest(QStringList() << "mw" << "srv" << "request" << "0"
                         << QString("client-auth,\"").append(slSettings.at(2)).append("\""));
}

void mainWindow::getServerIp()
{
    timerCheck->stop();
    serverStatus = 2;
    this->setServerStatus("connecting");
    QStringList slFtpData;
    slFtpData << "ftp://ftp.muzmarket.kz/srviplist" << "utilites@muzmarket.kz" << "47$Y@1hj0oo^3" << "21";

    QThread *tNetworkStatus = new QThread;
    networkStatus *nsw = new networkStatus(slFtpData);
    nsw->moveToThread(tNetworkStatus);
    connect(nsw,SIGNAL(setStatus(bool,QString)),this,SLOT(logger(bool,QString)));
    connect(nsw,SIGNAL(setIp(QByteArray)),this,SLOT(connectSocket(QByteArray)));
    connect(tNetworkStatus,SIGNAL(started()),nsw,SLOT(getSrvIp()),Qt::QueuedConnection);
    connect(nsw,SIGNAL(finished()),tNetworkStatus,SLOT(quit()));
    connect(tNetworkStatus,SIGNAL(finished()),nsw,SLOT(deleteLater()));
    connect(nsw,SIGNAL(destroyed()),tNetworkStatus,SLOT(deleteLater()));
    tNetworkStatus->start();
}

void mainWindow::setSocketProgressExpected(qint64 expected)
{
    pbProgress->setMaximum(expected);
    pbProgress->setValue(0);
}

void mainWindow::setSocketProgressCurrent(qint64 current)
{
    pbProgress->setValue(pbProgress->value() + current);
}

void mainWindow::getServerStatus()
{
    this->enqueueRequest(QStringList() << "mw" << "srv" << "request" << "0" << "checkonline");
}

void mainWindow::enqueueRequest(const QStringList &slRequestData)
{
    if(slRequestData.size()!=5)
    {this->logger(true,"Данные запроса повреждены");return;}

    QStringList slRequestStamp;
    QString sReqStamp = QDateTime::currentDateTime().toString("yy-MM-dd-HH-mm-ss-zzz");
    slRequestStamp << sReqStamp << slRequestData;
    lslRequestStamp.append(slRequestStamp);
    if(!socketIsOperating)
    {this->executeRequest();}
}

void mainWindow::executeRequest()
{
//    qDebug() << lslRequestStamp << "\n";
    this->fillRequestQueueModel();
    if (lslRequestStamp.size()==0)
    {return;}

    pbProgress->setValue(0);
    socketIsOperating = true;
    QString sSendData;
    sSendData.append(lslRequestStamp.at(0).at(2)).append(";;").append(lslRequestStamp.at(0).at(3)).append(";;")
            .append(lslRequestStamp.at(0).at(4)).append("\n").append(lslRequestStamp.at(0).at(5));
    this->sendData(lslRequestStamp.at(0).at(0),sSendData);
}

QString mainWindow::completeRequest(const QString &sReqStamp)
{
    QString sRet;
    for(int irow=0; irow<lslRequestStamp.size(); ++irow)
    {
        if(QString::compare(sReqStamp,lslRequestStamp.at(irow).at(0))==0)
        {sRet = lslRequestStamp.takeAt(irow).at(1);break;}
    }

    socketIsOperating = false;
    emit this->bufferAccepted();
    this->executeRequest();

    return sRet;
}


void mainWindow::fillRequestQueueModel()
{
    if(this->ui->tvw_requestQueue->model() != 0)
    {this->ui->tvw_requestQueue->model()->deleteLater();}
    QStandardItemModel *modelRequestQueue = new QStandardItemModel(this);
    modelRequestQueue->setHorizontalHeaderLabels(QStringList() << "Штамп" << "Цель");

    if(lslRequestStamp.isEmpty()){return;}

    for(int irow=0; irow<lslRequestStamp.size();++irow)
    {
        if (lslRequestStamp.at(irow).size() != 6)
        {this->logger(true,"Данные запроса повреждены");return;}

        QList<QStandardItem *> lsiModelRow;

        QStandardItem *siStamp = new QStandardItem(lslRequestStamp.at(irow).at(0));
        QStandardItem *siTarget = new QStandardItem(lslRequestStamp.at(irow).at(2));
        lsiModelRow.append(siStamp);
        lsiModelRow.append(siTarget);
        modelRequestQueue->appendRow(lsiModelRow);
    }
    if (modelRequestQueue->rowCount()>0)
    {
        modelRequestQueue->item(0,0)->setBackground(QBrush(Qt::lightGray));
        modelRequestQueue->item(0,1)->setBackground(QBrush(Qt::lightGray));
    }
    this->ui->tvw_requestQueue->setModel(modelRequestQueue);
    this->ui->tvw_requestQueue->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    this->ui->tvw_requestQueue->resizeColumnsToContents();
}

void mainWindow::sendData(const QString &sReqStamp, const QString &sSendData)
{
//    qDebug() << sReqStamp << sSendData;
    emit this->processClient(sReqStamp, sSendData.toStdString().c_str());
//    this->lockCurrentTab();
}

void mainWindow::resultFilter(const QStringList &slTarget, const QVector<QStringList> &vslResult)
{
    qDebug() << slTarget;
    if (QString::compare(slTarget.at(3),"1")==0)
    {qDebug() << vslResult.at(0);}
    QString sRet;
    if(QString::compare(slTarget.at(0),"local")==0)
    {this->processServerLocalRespond(slTarget,vslResult);return;}

    sRet = this->completeRequest(slTarget.at(0));

    if(QString::compare(sRet,"mw")==0)
    {
        if(QString::compare(slTarget.at(1),"srv")==0)
        {this->processServerRemoteRespond(slTarget, vslResult);}
        else if(QString::compare(slTarget.at(1),"db")==0)
        {this->processDbRespond(slTarget.at(2), vslResult);}
    }
    else
    {emit this->sendResult(slTarget.at(2), vslResult);}

}

void mainWindow::processServerLocalRespond(const QStringList &slTarget, const QVector<QStringList> &vslResult)
{
    if(QString::compare(slTarget.at(2),"connected")==0)
    {this->setServerStatus("connected");}
    else if(QString::compare(slTarget.at(2),"encrypted")==0)
    {
        timerConnect->stop();
        serverStatus = 1;
        this->setServerStatus("encrypted");
        this->authenticateClient();
    }
    else if(QString::compare(slTarget.at(2),"disconnected")==0)
    {
        this->logger(false,"Соединение разорвано");
        this->setServerStatus("offline");
        this->setClientStatus("offline");
    }
    else if(QString::compare(slTarget.at(2),"status")==0) {this->setServerStatus(vslResult.at(0).at(0));}
}

void mainWindow::processServerRemoteRespond(const QStringList &slTarget, const QVector<QStringList> &vslResult)
{
    if(QString::compare(vslResult.at(0).at(0),"closed")==0)
    {
        timerCheck->stop();
        disconnect(this,SIGNAL(processClient(QString,QByteArray)),scw,SLOT(processData(QString,QByteArray)));
        emit this->disconnectClient();
        tSslClient->quit();
    }
    else if(QString::compare(vslResult.at(0).at(0),"stop")==0)
    {
        this->logger(true,"Клиент не найден в базе сервера");
        this->setClientStatus("error");
    }
    else if(QString::compare(vslResult.at(0).at(0),"start")==0)
    {
        this->logger(false,"Клиент авторизован");
        this->setClientStatus("online");
    }
    else if(QString::compare(slTarget.at(2),"status")==0) {this->setServerStatus(vslResult.at(0).at(0));}
}

void mainWindow::setServerStatus(const QString &sStatus)
{
    QPalette palette = lblSrvStatus->palette();
    QColor color = Qt::red;
    if (QString::compare(sStatus,"online")==0)
    {
        serverStatus = 1;
        color = Qt::green;
    }
    else if (QString::compare(sStatus,"connecting")==0)
    {
        color = Qt::yellow;
        this->logger(false,"Соединяется...");
    }
    else if (QString::compare(sStatus,"connected")==0)
    {
        color = Qt::green;
        this->logger(false,"Соединение с сервером установлено");
    }
    else if (QString::compare(sStatus,"encrypted")==0)
    {
        color = Qt::blue;
        this->logger(false,"Зашифрованное соединение установлено");
    }
    else if (QString::compare(sStatus,"offline")==0)
    {
        this->setClientStatus(sStatus);
        serverStatus = 0;
        color = Qt::red;
        timerConnect->start();
    }
    palette.setColor(lblSrvStatus->backgroundRole(), color);
    lblSrvStatus->setPalette(palette);
}

void mainWindow::setClientStatus(const QString &sStatus)
{
    QPalette palette = lblClientStatus->palette();
    QColor color = Qt::red;
    if (QString::compare(sStatus,"online")==0)
    {
        timerCheck->start();
        clientStatus = 1;
        color = Qt::green;
    }
    else if (QString::compare(sStatus,"error")==0)
    {
        clientStatus = 2;
        color = Qt::yellow;
    }
    else if (QString::compare(sStatus,"offline")==0)
    {
        clientStatus = 0;
        color = Qt::red;
    }
    palette.setColor(lblClientStatus->backgroundRole(), color);
    lblClientStatus->setPalette(palette);
}

void mainWindow::processDbRespond(const QString &sTarget, const QVector<QStringList> &vslResult)
{
//    qDebug() << sTarget;
//    qDebug() << vslResult;
    QStringList slTarget = sTarget.split("::");

    if(QString::compare(slTarget.at(0),"depotlist")==0) {this->fillCbDepots(vslResult);}
    else if(QString::compare(slTarget.at(0),"producttypeslist")==0) {this->fillCbRegTypes(vslResult);}
    else if(QString::compare(slTarget.at(0),"registrylist")==0) {this->fillRegistryModel(vslResult);}
    else if(QString::compare(slTarget.at(0),"goodslist")==0) {this->fillRetailModel(vslResult);}
}

void mainWindow::fillCbDepots(const QVector<QStringList> &vslResult)
{
    this->ui->cb_depots->clear();
//    this->ui->cb_depots->addItem("весь товар");
    for (int iitm=0; iitm<vslResult.size(); ++iitm)
    {this->ui->cb_depots->addItem(vslResult.at(iitm).at(0));}
}

void mainWindow::fillCbRegTypes(const QVector<QStringList> &vslResult)
{
    this->ui->cb_registry_types->clear();
    for (int iitm=0; iitm<vslResult.size(); ++iitm)
    {this->ui->cb_registry_types->addItem(vslResult.at(iitm).at(0));}
}

void mainWindow::fillTableInvoice()
{
//    QStandardItemModel *modelIn= new QStandardItemModel(this);
//    if(modelIn!=0) {delete modelIn; modelIn=0;}

//    for (int irow=0; irow<vslTable.size(); ++irow)
//    {
//        QStandardItem *siItmName = new QStandardItem(vslTable.at(irow).at(0));
//        QStandardItem *siVendor = new QStandardItem(vslTable.at(irow).at(1));
//        QStandardItem *siUnit = new QStandardItem(vslTable.at(irow).at(2));
//        QStandardItem *siSeries = new QStandardItem(vslTable.at(irow).at(3));
//        QStandardItem *siExpDate = new QStandardItem(vslTable.at(irow).at(4));
//        QStandardItem *siPrice = new QStandardItem(vslTable.at(irow).at(5));
//        QStandardItem *siQuantity = new QStandardItem(vslTable.at(irow).at(6));

//        if (QDate::fromString(siExpDate->text(),"dd.MM.yyyy") <= QDate::currentDate().addMonths(1))
//        {siExpDate->setBackground(Qt::red);}

//        QList<QStandardItem *> lsiModelRow;
//        lsiModelRow.append(siItmName);
//        lsiModelRow.append(siVendor);
//        lsiModelRow.append(siUnit);
//        lsiModelRow.append(siSeries);
//        lsiModelRow.append(siExpDate);
//        lsiModelRow.append(siPrice);
//        lsiModelRow.append(siQuantity);
//        modelIn->appendRow(lsiModelRow);
//    }
//    modelIn->setHorizontalHeaderLabels(QStringList() << "Наименование" << "Производитель"
//                                       << "Ед. изм." << "Серия" << "Срок годности" << "Цена" << "К-во");

//    ui->tvw_inProc->setModel(modelIn);
//    int effWidth = ui->tvw_inProc->width();
//    ui->tvw_inProc->setColumnWidth(0, effWidth * 0.3);
//    ui->tvw_inProc->setColumnWidth(1, effWidth * 0.24);
//    ui->tvw_inProc->setColumnWidth(2, effWidth * 0.05);
//    ui->tvw_inProc->setColumnWidth(3, effWidth * 0.1);
//    ui->tvw_inProc->setColumnWidth(4, effWidth * 0.1);
//    ui->tvw_inProc->setColumnWidth(5, effWidth * 0.1);
    //    ui->tvw_inProc->setColumnWidth(6, effWidth * 0.05);
}

void mainWindow::fillRetailModel(const QVector<QStringList> &vslResult)
{
//    QStandardItemModel *modelResult = new QStandardItemModel(this);
//    modelResult->setHorizontalHeaderLabels(QStringList() << "Код" << "Серия" << "Наименование" << "Производитель"
//                                       << "Ед. изм." << "Срок годности" << "Штрихкод" << "Поставщик"
//                                         << "№ накладной" << "Дата накладной" << "К-во" << "Цена закупки"
//                                         << "Фактор"  << "Цена продажи"  << "Дата");
//    QStringList slItems;

//    for (int irow = 2; irow < vslResult.size(); ++irow)
//    {
//        slItems = vslResult.at(irow).split("--");
//        if(slItems.size()==1 && vslResult.size()==3) {continue;}
//        if(slItems.size() != 15)
//        {this->logger(true,"Ошибка в полученных данных");return;}

//        QStandardItem *siCode = new QStandardItem();
//        siCode->setData(slItems.at(0).toInt(),Qt::DisplayRole);
//        QStandardItem *siSeries = new QStandardItem(slItems.at(1));
//        QStandardItem *siName = new QStandardItem(slItems.at(2));
//        QStandardItem *siManufactorer = new QStandardItem(slItems.at(3));
//        QStandardItem *siUnit = new QStandardItem(slItems.at(4));
//        QStandardItem *siExpDate = new QStandardItem(slItems.at(5));
//        QStandardItem *siBarcode = new QStandardItem(slItems.at(6));
//        QStandardItem *siVendor = new QStandardItem(slItems.at(7));
//        QStandardItem *siInvoiceNum = new QStandardItem(slItems.at(8));
//        QStandardItem *siInvoiceDate = new QStandardItem(slItems.at(9));
//        QStandardItem *siQuantity = new QStandardItem();
//        siQuantity->setData(slItems.at(10).toInt(),Qt::DisplayRole);
//        QStandardItem *siPurchPrice = new QStandardItem();
//        siPurchPrice->setData(slItems.at(11).toDouble(),Qt::DisplayRole);
//        QStandardItem *siFactor = new QStandardItem(slItems.at(12));
//        QStandardItem *siSellPrice = new QStandardItem();
//        siSellPrice->setData(slItems.at(13).toDouble(),Qt::DisplayRole);
//        QStandardItem *siProcStamp = new QStandardItem(slItems.at(14));

//        QList<QStandardItem *> lsiModelRow;
//        lsiModelRow.append(siCode);
//        lsiModelRow.append(siSeries);
//        lsiModelRow.append(siName);
//        lsiModelRow.append(siManufactorer);
//        lsiModelRow.append(siUnit);
//        lsiModelRow.append(siExpDate);
//        lsiModelRow.append(siBarcode);
//        lsiModelRow.append(siVendor);
//        lsiModelRow.append(siInvoiceNum);
//        lsiModelRow.append(siInvoiceDate);
//        lsiModelRow.append(siQuantity);
//        lsiModelRow.append(siPurchPrice);
//        lsiModelRow.append(siFactor);
//        lsiModelRow.append(siSellPrice);
//        lsiModelRow.append(siProcStamp);
//        modelResult->appendRow(lsiModelRow);
//    }

//    ui->tvw_report_goods->setModel(modelResult);
}

void mainWindow::fillRegistryModel(const QVector<QStringList> &vslResult)
{
    if(this->ui->tvw_report_regisry->model() != 0)
    {this->ui->tvw_report_regisry->model()->deleteLater();}
    QStandardItemModel *modelRegistry = new QStandardItemModel(this);
    modelRegistry->setHorizontalHeaderLabels(vslResult.at(0));

    for (int irow = 0; irow < vslResult.size(); ++irow)
    {
//        QStringList slItems = vslResult.at(irow).split("--");
//        qDebug() << slItems;
//        qDebug() << slItems.size() << slHeaderLabels.size();
        if (vslResult.at(irow).size() != vslResult.at(0).size())
        {this->logger(true,"Данные повреждены");return;}

        QList<QStandardItem *> lsiModelRow;
        for(int jcol = 0; jcol < vslResult.at(irow).size(); ++jcol)
        {QStandardItem *siCurrent = new QStandardItem(vslResult.at(irow).at(jcol)); lsiModelRow.append(siCurrent);}
        modelRegistry->appendRow(lsiModelRow);
    }
    this->ui->tvw_report_regisry->setModel(modelRegistry);
}

void mainWindow::logger(bool stType, QString sLine)
{
    if(this->ui->lW_client->count()>500)
    {
        for(int iitm=0;iitm<this->ui->lW_client->count()-500;++iitm)
        {this->ui->lW_client->takeItem(iitm);}
    }
    this->ui->lW_client->addItem(QTime::currentTime().toString().append(" ").append(sLine));
    this->ui->lW_client->scrollToBottom();

    if (stType==true)
    {QMessageBox::critical(this,"Ошибка",sLine,QMessageBox::Ok);}
}

void mainWindow::lockCurrentTab()
{
    this->ui->tw_main->setTabEnabled(this->ui->tw_main->currentIndex(),false);
}

void mainWindow::unlockCurrentTab()
{
    this->ui->tw_main->setTabEnabled(this->ui->tw_main->currentIndex(),true);
}

void mainWindow::on_btn_processIn_clicked()
{
//    this->sendData("mw","db;;insert;;wh", xlVector);
}

void mainWindow::on_btn_addOut_clicked()
{

}

void mainWindow::on_btn_deleteOut_clicked()
{

}

void mainWindow::on_btn_processOut_clicked()
{

}

void mainWindow::on_btn_selectRetail_clicked()
{
    if (this->ui->cb_depots->currentText().isEmpty())
    {emit this->logger(1,"Не выбран склад");return;}
    this->enqueueRequest(QStringList() << "mw" << "db" << "request" << "0"
                         << QString("select,retail,").append(this->ui->cb_depots->currentText()).append(",all"));
}

void mainWindow::on_cb_vendors_activated(int index)
{
    QPointer<files> fw = new files(this);
    connect(fw, SIGNAL(setStatus(bool,QString)), this, SLOT(logger(bool,QString)));
    QString sFileName = QFileDialog::getOpenFileName(this, tr("Открыть файл"), slSettings.at(0), tr("Файлы Excel (*.xls *.xlsx)"));
    vslTable = fw->processXlData(sFileName, lslLibInfo.at(index).at(2));
    delete fw;
    this->fillTableInvoice();
}

void mainWindow::on_act_exit_triggered()
{
    this->close();
}

void mainWindow::on_act_settings_triggered()
{
    QPointer <dlg_settings> dsw = new dlg_settings(this);
    dsw->setAttribute(Qt::WA_DeleteOnClose);
    dsw->setModal(true);
    dsw->show();
}

void mainWindow::on_btn_getDepotList_clicked()
{
    this->enqueueRequest(QStringList() << "mw" << "db" << "request" << "0" << "select,lists,depots,all");
}

void mainWindow::on_cb_depots_currentTextChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {this->ui->btn_selectRetail->setEnabled(false);}
    else {this->ui->btn_selectRetail->setEnabled(true);}
}

void mainWindow::on_btn_addRecord_clicked()
{
//    QPointer <dlg_addProductRecord> daprw = new dlg_addProductRecord(this);
//    daprw->setAttribute(Qt::WA_DeleteOnClose);
//    connect(daprw,SIGNAL(sendRequest(QString,QString,QStringList)),this,SLOT(sendData(QString,QString,QStringList)));
//    connect(this,SIGNAL(sendResult(QVector<QStringList>)),daprw,SLOT(handleResult(QVector<QStringList>)));
//    daprw->setModal(true);
//    daprw->show();
//    daprw->setInitialData();
}

void mainWindow::on_tvw_inProc_doubleClicked(const QModelIndex &index)
{
//    QPointer<dlg_addWarehouseRecord> dawrw = new dlg_addWarehouseRecord(this);
//    dawrw->setAttribute(Qt::WA_DeleteOnClose);
//    connect(dawrw,SIGNAL(sendRequest(QString,QString,QStringList)),this,SLOT(sendData(QString,QString,QStringList)));
//    connect(this,SIGNAL(sendResult(QVector<QStringList>)),dawrw,SLOT(handleResult(QVector<QStringList>)));
//    dawrw->setModal(true);
//    dawrw->show();
//    dawrw->setInitialData(QStringList() << modelIn->item(index.row(),0)->text() <<
//                         modelIn->item(index.row(),1)->text() << modelIn->item(index.row(),2)->text() <<
//                         modelIn->item(index.row(),3)->text() << modelIn->item(index.row(),4)->text() <<
//                         modelIn->item(index.row(),5)->text() << modelIn->item(index.row(),6)->text());
}

void mainWindow::on_btn_getRegistryTypes_clicked()
{
    this->enqueueRequest(QStringList() << "mw" << "db" << "request" << "0" << "select,lists,producttypes,all");
}

void mainWindow::on_btn_selectRegistry_clicked()
{
    this->enqueueRequest(QStringList() << "mw" << "db" << "request" << "0" << QString("select,registry,\"")
                   .append(this->ui->cb_registry_types->currentText()).append("\",all"));
}

void mainWindow::on_btn_registerProduct_clicked()
{
    QPointer <dlg_addProduct> dap = new dlg_addProduct(this);
    dap->setAttribute(Qt::WA_DeleteOnClose);
    connect(dap,SIGNAL(sendRequest(QStringList)),this,SLOT(enqueueRequest(QStringList)));
    connect(this,SIGNAL(sendResult(QString,QVector<QStringList>)),dap,SLOT(handleResult(QString,QVector<QStringList>)));
//    connect(dap,SIGNAL(setStatus(bool,QString)),this,SLOT(logger(bool,QString)));
    dap->setModal(true);
    dap->show();
}

void mainWindow::on_btn_alterProductData_clicked()
{
    QPointer <dlg_alterProductData> dapd = new dlg_alterProductData(this);
    dapd->setAttribute(Qt::WA_DeleteOnClose);
    dapd->setModal(true);
    dapd->show();
}

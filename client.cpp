#include "client.h"

sslClient::sslClient(QObject *parent) : QObject(parent)
{
    sslSocket = new QSslSocket(this);
    connect(sslSocket,SIGNAL(connected()),this,SLOT(socketConnected()));
    connect(sslSocket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()));
    connect(sslSocket,SIGNAL(readyRead()),this,SLOT(socketReadyRead()));
    connect(sslSocket,SIGNAL(encrypted()),this,SLOT(socketEncrypted()));
    connect(sslSocket,SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError(QList<QSslError>)));
//    _clientID = QString("1").toStdString().c_str();
}

sslClient::~sslClient()
{
    sslSocket->deleteLater();
    emit this->finished();
}

bool sslClient::networkIsUp()
{
//    bool activemac("0");
    QList<QNetworkInterface> iflist = QNetworkInterface::allInterfaces();
    if (iflist.isEmpty()) {return false;}

    for (int iface = 0; iface < iflist.size(); ++iface)
    {if (QNetworkInterface::interfaceFromIndex(iface).isValid()) {return true;}}
    return false;
}

void sslClient::connectToServer(const QString &sReqStamp, const QString &sServerIp)
{
    sRequestStamp = sReqStamp;
    if (!this->networkIsUp())
    {this->socketError("сети нет, мы все умрём");return;}

    if (!sslSocket->supportsSsl())
    {this->socketError("ssl не поддерживается");return;}

//    emit this->setResult(QByteArray("srv\nstatus\nconnecting"));
    sslSocket->connectToHostEncrypted("127.0.0.1",10742);
//    sslSocket->connectToHostEncrypted("192.168.1.100",10742);
//    sslSocket->connectToHostEncrypted(sServerIp,10742);
    if(!sslSocket->waitForConnected())
    {emit this->socketError(sslSocket->errorString());return;}
    sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
    sslSocket->ignoreSslErrors();

    if(!sslSocket->waitForEncrypted())
    {emit this->socketError(sslSocket->errorString());return;}
}

void sslClient::disconnectFromServer()
{
    sslSocket->close();
}

void sslClient::processData(const QString &sReqStamp, const QByteArray &baData)
{
    sRequestStamp = sReqStamp;
//    qDebug() << baData;
    if (this->sendBuffer(baData)!=0)
    {this->socketError("Ошибка передачи данных");return;}

    if (this->getBuffer()!=0)
    {this->socketError("Ошибка приёма данных");return;}
//    qDebug() << baBuffer;

    emit this->setStatus(false, "task done");

    QByteArray baTmp;
    for (int iitm=0; iitm<baBuffer.size();)
    {
        baTmp.append(baBuffer.at(iitm));
        baBuffer.remove(iitm,1);
        if (baBuffer.at(iitm) == '\n')
        {baBuffer.remove(iitm,1); break;}
    }

    QStringList slParams;
    slParams << sRequestStamp << QString(baTmp).split(";;");

    emit this->setResult(slParams, this->parseCsv(baBuffer));
    emit this->processFinished();
}

int sslClient::sendBuffer(const QByteArray &baSend)
{
    QByteArray baSize = QByteArray::number(baSend.size());
    if (baSize.size() > 4096)
    {this->socketError("Объём запроса слишком велик");return 1;}

    sslSocket->write(sRequestStamp.toStdString().c_str());
    if(!sslSocket->waitForBytesWritten())
    {this->serverTimeout(); return 1;}
    if(!sslSocket->waitForReadyRead())
    {this->serverTimeout();return 1;}

    QString sReqStamp = sslSocket->readAll();

    if (QString::compare(sReqStamp,sRequestStamp)!=0)
    {this->socketError("Ошибка синхронизации штампов запросов");return 1;}

    sslSocket->write(baSize);
    if(!sslSocket->waitForBytesWritten())
    {this->serverTimeout(); return 1;}
    bytesExpected = 0;
    if(!sslSocket->waitForReadyRead())
    {this->serverTimeout();return 1;}

    bytesExpected = sslSocket->readAll().toLongLong();

    if (bytesExpected != baSend.size())
    {this->socketError("Ошибка передачи данных");return 1;}

    sslSocket->write(baSend);
    if(!sslSocket->waitForBytesWritten())
    {this->serverTimeout();return 1;}
    return 0;
}

int sslClient::getBuffer()
{
    baBuffer.clear();
    if(!sslSocket->waitForReadyRead())
    {this->serverTimeout();return 1;}
    baBuffer = sslSocket->readAll();
    if (QString::compare(baBuffer,sRequestStamp)!=0)
    {
        sslSocket->write("-");
        this->socketError("Ошибка синхронизации штампов запросов");
        return 1;
    }
    sslSocket->write(baBuffer);
    if(!sslSocket->waitForBytesWritten())
    {this->serverTimeout();return 1;}

    baBuffer.clear();
    bytesExpected = 0;
    if(!sslSocket->waitForReadyRead())
    {this->serverTimeout();return 1;}
    baBuffer = sslSocket->readAll();
    bytesExpected = baBuffer.toLongLong();
    emit this->socketProgressExpected(bytesExpected);
    sslSocket->write(baBuffer);
    if(!sslSocket->waitForBytesWritten())
    {this->serverTimeout();return 1;}
    baBuffer.clear();

    while (bytesExpected != 0)
    {
        if (!sslSocket->waitForReadyRead(5000))
        {return 1;}

        qint64 toRead = qMin(bytesExpected,sslSocket->bytesAvailable());
        QByteArray baChunk = sslSocket->read(toRead);
        baBuffer.append(baChunk);
        bytesExpected -= baChunk.size();

        if (bytesExpected < 0)
        {
            baBuffer.clear();
            bytesExpected=0;
            return 1;
        }
        emit this->socketProgressCurrent(toRead);
    }
    return 0;
}

QVector<QStringList> sslClient::parseCsv(QByteArray baCsv)
{
    QString sCsv(baCsv);
    baCsv.clear();

    csvReader *crw = new csvReader;
    return crw->parseCsv(sCsv);
}

void sslClient::socketConnected()
{
    QVector<QStringList> vslResult;
    emit this->setResult(QStringList() << sRequestStamp << "srv" << "connected" << "0", vslResult);
}

void sslClient::socketEncrypted()
{
    QVector<QStringList> vslResult;
    emit this->setResult(QStringList() << sRequestStamp << "srv" << "encrypted" << "0", vslResult);
}

void sslClient::socketReadyRead()
{
    emit this->setStatus(false, QString("Reading: ").append(QString::number(sslSocket->bytesAvailable())));
}

void sslClient::socketDisconnected()
{
    QVector<QStringList> vslResult;
    emit this->setResult(QStringList() << "local" << "srv" << "disconnected" << "0", vslResult);
}

void sslClient::sslError(const QList<QSslError> &lseError)
{
    this->socketError(lseError.at(lseError.size()-1).errorString());
}

void sslClient::clearBuffer()
{
    baBuffer.clear();
}

void sslClient::socketError(const QString &sError)
{
    QVector<QStringList> vslResult;
    vslResult.append(QStringList() << "offline" << sError);
    emit this->setResult(QStringList()  << sRequestStamp << "srv" << "status" << "1", vslResult);
}

void sslClient::serverTimeout()
{
    this->socketError("Превышено ограничение ожидания ответа от сервера");
    sslSocket->close();
}

//-----------------------------------------------------------------------------------------------

networkStatus::networkStatus(const QStringList &slFtpData)
{
    _slFtpData = slFtpData;
//    ftpData.clear();
}

networkStatus::~networkStatus()
{
}

void networkStatus::getSrvIp()
{
    QList<QNetworkInterface> lIf = QNetworkInterface::allInterfaces();
    if (lIf.isEmpty())
    {emit this->setStatus(true,"iflist is empty"); return;}

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);

    QUrl url(_slFtpData.at(0));
    url.setUserName(_slFtpData.at(1));
    url.setPassword(_slFtpData.at(2));
    url.setPort(_slFtpData.at(3).toInt());
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Anonymous");
    rpl = nam->get(request);
    connect(rpl,SIGNAL(readyRead()),this,SLOT(rplReadyRead()));
    connect(rpl,SIGNAL(finished()),this,SLOT(rplFinished()));
    connect(rpl,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(replyError(QNetworkReply::NetworkError)));
    connect(rpl,SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(sslErrors(QList<QSslError>)));
}

void networkStatus::replyError(const QNetworkReply::NetworkError &neState)
{
    emit this->setStatus(true,QString::number(neState));
}

void networkStatus::sslErrors(const QList<QSslError> &errList)
{
    for (int iitm=0;iitm<errList.size();iitm++)
    {emit this->setStatus(true,errList.at(iitm).errorString());}
}


void networkStatus::rplReadyRead()
{
    baSrvIp.append(rpl->readAll());
}

void networkStatus::rplFinished()
{
    emit this->setStatus(false,"Файл настроек сервера получен");
    emit this->setIp(baSrvIp);
    emit this->finished();
    rpl->deleteLater();
    return;
}

//------------------------------------------------------------------------------------------------------

csvReader::csvReader()
{

}

csvReader::~csvReader()
{

}

QVector<QStringList> csvReader::parseCsv(QString sData)
{
    sData.remove(QRegExp("\r")); //remove all ocurrences of CR (Carriage Return)
    QString sTemp;
    QChar cCurrent;
    QTextStream tsIn(&sData);
    while (!tsIn.atEnd())
    {
        tsIn >> cCurrent;
        if (cCurrent == ',')
        {this->checkString(sTemp, cCurrent);}
        else if (cCurrent == '\n')
        {this->checkString(sTemp, cCurrent);}
        else if (tsIn.atEnd())
        {sTemp.append(cCurrent);this->checkString(sTemp);}
        else
        {sTemp.append(cCurrent);}
    }
    return vslOut;
}

void csvReader::checkString(QString &sTemp, QChar cCurrent)
{
    if(sTemp.count("\"")%2 == 0)
    {
//        if (sTemp.size() == 0 && cCurrent != ',') //problem with line endings
//        {return;}
        if (sTemp.startsWith(QChar('\"')) && sTemp.endsWith(QChar('\"')))
        {
             sTemp.remove(QRegExp("^\""));
             sTemp.remove(QRegExp("\"$"));
        }
        //FIXME: will possibly fail if there are 4 or more reapeating double quotes
        sTemp.replace("\"\"", "\"");
        slOut.append(sTemp);
        if (cCurrent != QChar(','))
        {vslOut.append(slOut); slOut.clear();}
        sTemp.clear();
    }
    else
    {sTemp.append(cCurrent);}
}

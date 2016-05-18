#ifndef CLIENT_H
#define CLIENT_H

#include <QDir>
#include <QSslKey>
#include <QSslSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class sslClient : public QObject
{
    Q_OBJECT

    QSslSocket *sslSocket;

    QString _sServerIp;
    QString sRequestStamp;

    int expSize;
    qint64 bytesExpected;
    QByteArray baBuffer;

public:
    explicit sslClient(QObject *parent = 0);
    virtual ~sslClient();

signals:
    void setStatus(bool stType, const QString &sError);
    void socketProgressExpected(qint64 expected);
    void socketProgressCurrent(qint64 current);
    void setResult(const QStringList &slTarget, const QVector<QStringList> &vslOut);
    void processFinished();
    void finished();

private slots:
    bool networkIsUp();
    int sendBuffer(const QByteArray &baSend);
    int getBuffer();
    void socketError(const QString &sError);
    void serverTimeout();
    
public slots:
    void connectToServer(const QString &sReqStamp, const QString &sServerIp);
    void disconnectFromServer();
    void processData(const QString &sReqStamp, const QByteArray &baData);
    void socketConnected();
    void socketEncrypted();
    void socketReadyRead();
    void socketDisconnected();
    void sslError(const QList<QSslError> &lseError);
    void clearBuffer();
    QVector<QStringList> parseCsv(QByteArray baCsv);
};

//------------------------------------------------------------------------------------------------------

class networkStatus : public QObject
{
    Q_OBJECT

    QStringList _slFtpData;
    QNetworkReply *rpl;
    QByteArray baSrvIp;

public:
    explicit networkStatus(const QStringList &slFtpData);
    virtual ~networkStatus();

signals:
    void setStatus(const bool &stType, const QString &sStatus);
    void setIp(const QByteArray &baSrvIp);
    void finished();

private slots:
    void rplFinished();
    void rplReadyRead();
    void replyError(const QNetworkReply::NetworkError &neState);
    void sslErrors(const QList<QSslError> &errList);

public slots:
    void getSrvIp();

};

//------------------------------------------------------------------------------------------------------

class csvReader : public QObject
{
    Q_OBJECT

    QVector<QStringList> vslOut;
    QStringList slOut;

public:
    explicit csvReader();
    virtual ~csvReader();

private slots:
    void checkString(QString &sTemp, QChar cCurrent = 0);

public slots:
    QVector<QStringList> parseCsv(QString sData);
};

//------------------------------------------------------------------------------------------------------

#endif // CLIENT_H

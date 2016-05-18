#ifndef FILES_H
#define FILES_H

#include <QFileDialog>
#include <QTextStream>
#include <QDate>
#include <QLibrary>
#include <QDebug>

#include "freexl.h"
#include "xlsxdocument.h"
#include "xlsxcellrange.h"

class files : public QObject
{
    Q_OBJECT

    QString sSettingsFileName;
    QString sServerIpFileName;

public:
    explicit files(QWidget *parent = 0);
    virtual ~files();

    QStringList readSettings();
    void writeFile(const QString &sOut, const QString &sFileName);
    void writeSettings(const QStringList &slSettings);
    void writeServerIp(const QString &sSrvIp);
    QVector<QStringList> getXlsData(const QString &sFpath);
    QVector<QStringList> getXlsxData(const QString &sFpath);
    QList<QStringList> loadProcLibs(const QString &sProcLibsDir);
    QStringList getProcLibInfo(const QString &sLibName);
    QVector<QStringList> processXlData(const QString &sFileName, const QString &sLibName);
    QString getServerIp();

signals:
    void setStatus(const bool &stType, const QString &sLine);
};

#endif // FILES_H

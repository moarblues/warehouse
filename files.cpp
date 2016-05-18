#include "files.h"

files::files(QWidget *parent) : QObject(parent)
{
    sSettingsFileName = QDir::currentPath().append("/settings.conf");
    sServerIpFileName = QDir::currentPath().append("/common/srviplist");
}

files::~files()
{
}

QStringList files::readSettings()
{
    QStringList slOut;
    QFile fSettings(sSettingsFileName);
    if (fSettings.open(QFile::ReadOnly))
    {
//        const QString &errWS("Путь каталога или значение не должны содержать символа ");
        QString sFline;

        QTextStream tsIn(&fSettings);
        tsIn.setCodec("UTF-8");
        do
        {
            sFline = tsIn.readLine();

            if (sFline.isEmpty() || QString::compare(sFline.at(0),"#") == 0)
            {continue;}

            slOut.append(sFline);
            continue;
        } while (!sFline.isNull());
    }
    return slOut;
}

void files::writeFile(const QString &sOut, const QString &sFileName)
{
    QFile *fOut = new QFile(this);
    QTextStream tsOut(fOut);

    fOut->setFileName(sFileName);
    tsOut.setCodec("UTF-8");
    if (fOut->open(QFile::WriteOnly))
    {tsOut << sOut;}
    fOut->flush();
    fOut->close();
    delete fOut;
}

void files::writeSettings(const QStringList &slSettings)
{
    QFile *fEx = new QFile(this);
    QString sTmpName, sOut;

    fEx->setFileName(sSettingsFileName);
    sTmpName = sSettingsFileName;
    sTmpName.append(".tmp");

    for (int irow = 0; irow < slSettings.size(); irow++)
    {
        if (irow == 0) {sOut.append("#proclibs\n");}
        if (irow == 2) {sOut.append("\n#network\n");}

        QString sTemp = slSettings.at(irow);
        foreach (QChar cStritm, sTemp)
        {
            if (cStritm == '\\')
            {sOut.append("\\\\");}
            else
            {sOut.append(cStritm);}
        }
        sOut.append("\n");
    }

    fEx->close();
    delete fEx;
    this->writeFile(sOut, sTmpName);
    QFile::remove(sSettingsFileName);
    QFile::rename(sTmpName,sSettingsFileName);
}

void files::writeServerIp(const QString &sSrvIp)
{
    this->writeFile(sSrvIp, sServerIpFileName);
}

QVector<QStringList> files::getXlsData(const QString &sFpath)
{
    unsigned int worksheet_index;
    unsigned int max_worksheet;
    unsigned int info;
    int ret;
    const void *handle;
    unsigned int rows;
    unsigned short columns;
    unsigned int row;
    unsigned short col;

    QVector<QStringList> outVector;
    QString cellstr;

    // opening the .XLS file [Workbook]
    ret = freexl_open (sFpath.toStdString().c_str(), &handle);
    if (ret != FREEXL_OK)
    {
        emit this->setStatus(true, QString("Ошибка открытия: ").append(QString::number(ret)));
        return outVector;
    }

    // checking for Password (obfuscated/encrypted)
    ret = freexl_get_info (handle, FREEXL_BIFF_PASSWORD, &info);
    if (ret != FREEXL_OK)
    {
        emit this->setStatus(true, QString("Ошибка получения информации (необходим пароль) ")
                .append(QString::number(FREEXL_BIFF_PASSWORD)).append(": ").append(QString::number(ret)));
        return outVector;
    }
    switch (info)
    {
        case FREEXL_BIFF_PLAIN:
            break;
        case FREEXL_BIFF_OBFUSCATED:
        default:
            emit this->setStatus(true, QString("Защищено паролем, нет доступа"));
            return outVector;
    };

    // querying BIFF Worksheet entries
    ret = freexl_get_info (handle, FREEXL_BIFF_SHEET_COUNT, &max_worksheet);
    if (ret != FREEXL_OK)
    {
        emit this->setStatus(true, QString("Ошибка получения информации (ошибка чтения листов)")
               .append(FREEXL_BIFF_SHEET_COUNT).append(": ").append(QString::number(ret)));
        return outVector;
    }

    for (worksheet_index = 0; worksheet_index < max_worksheet; worksheet_index++)
    {
        const char *utf8_worsheet_name;
        ret = freexl_get_worksheet_name (handle, worksheet_index, &utf8_worsheet_name);
        if (ret != FREEXL_OK)
        {
            emit this->setStatus(true, QString("Ошибка чтения листов: ")
                                 .append(QString::number(ret)));
            return outVector;
         }
    // selecting the active Worksheet
        ret = freexl_select_active_worksheet (handle, worksheet_index);
        if (ret != FREEXL_OK)
        {
            emit this->setStatus(true, QString("Ошибка выбора текущего листа: ").append(QString::number(ret)));
            return outVector;
        }
    // dimensions
        ret = freexl_worksheet_dimensions (handle, &rows, &columns);
        if (ret != FREEXL_OK)
        {
            emit this->setStatus(true, QString("Ошибка структуры листов: ")
                                 .append(QString::number(ret)));
            return outVector;
        }

        for (row = 0; row < rows; row++)
        {
            QStringList recordRow;
            FreeXL_CellValue cell;
            for (col = 0; col < columns; col++)
            {
                ret = freexl_get_cell_value (handle, row, col, &cell);
                if (ret != FREEXL_OK)
                {
                    emit this->setStatus(true, QString("Ошибка значения ячейки (строка=")
                                         .append(row).append(" столбец=").append(col).append("): ")
                                         .append(QString::number(ret)));
                    return outVector;
                }
                switch (cell.type)
                {
                case FREEXL_CELL_INT:
                    cellstr = QString::number(cell.value.int_value);
                    break;
                case FREEXL_CELL_DOUBLE:
                    cellstr = QString::number(cell.value.double_value);
                    break;
                case FREEXL_CELL_TEXT:
                case FREEXL_CELL_SST_TEXT:
                    cellstr = cell.value.text_value;
                    break;
                case FREEXL_CELL_DATE:
                case FREEXL_CELL_DATETIME:
                case FREEXL_CELL_TIME:
                    cellstr = cell.value.text_value;
                    break;
                case FREEXL_CELL_NULL:
                    cellstr = "";
                default:
                    break;
                };
                recordRow.append(cellstr.simplified());
            }
//            if (recordRow.at(0).isEmpty()) {continue;}
            outVector.append(recordRow);
        }
    }
    ret = freexl_close(handle);
    if (ret != FREEXL_OK)
    {
        emit this->setStatus(true, QString("Ошибка закрытия: ").append(QString::number(ret)));
        return outVector;
    }
    return outVector;
}

QVector<QStringList> files::getXlsxData(const QString &sFpath)
{
    QXlsx::Document xlsxentry(sFpath);
    QVector<QStringList> outVector;

//    QString errStr;

    for (int irow=0; irow<xlsxentry.dimension().rowCount(); irow++)
    {
        QStringList recordRow;

        for(int jcol=0; jcol<xlsxentry.dimension().columnCount(); jcol++)
        {recordRow.append(xlsxentry.read(irow,jcol).toString().simplified());}

        if (recordRow.at(0).isEmpty()) {continue;}
        outVector.append(recordRow);
        continue;
    }
    return outVector;
}

QList<QStringList> files::loadProcLibs(const QString &sProcLibsDir)
{
    QList<QStringList> lslLibInfo;
    QStringList slProcLibs;

    QDir dProcLibs(sProcLibsDir);
    const QStringList sNameFilter("*.so");
    slProcLibs = dProcLibs.entryList(sNameFilter, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    for (int procsCount = 0; procsCount < slProcLibs.count(); procsCount++)
    {lslLibInfo.append(this->getProcLibInfo(QString(sProcLibsDir).append('/').append(slProcLibs.at(procsCount))));}
    return lslLibInfo;
}

QStringList files::getProcLibInfo(const QString &sLibName)
{
    QStringList slLibInfo;
    if (!sLibName.isNull() && !sLibName.isEmpty())
    {}
    QLibrary libProc(sLibName);
    if(!libProc.load())
    {
        // .so load error here
        this->setStatus(false, QString("Не удалось загрузить библиотеку обработки\n").append(sLibName));
        return slLibInfo << "" << "" << sLibName;
    }

    typedef QStringList (*iM)();
    iM ims = (iM) libProc.resolve("getInfo");
    if (ims)
    {
        slLibInfo << ims() << sLibName;
//        "Поставщик" << "Версия" << "Путь к библиотеке";
    }
    else
    {
       // .so resolve error here here
        this->setStatus(false, QString("Не удалось получить информацию из библиотеки обработки\n")
                        .append(sLibName));
        slLibInfo << "" << "" << sLibName;
    }
    libProc.unload();

    return slLibInfo;
}

QVector<QStringList> files::processXlData(const QString &sFileName, const QString &sLibName)
{
    QVector<QStringList> vslOut;
    QVector<QStringList> vslRaw;
    if (sLibName.isNull() && sLibName.isEmpty())
    {this->setStatus(true, "Ошибка библиотеки обработки\nОбновите список библиотек"); return vslOut;}

    QLibrary libProc(sLibName);
    if(!libProc.load())
    {
        // .so load error here
        this->setStatus(true, QString("Не удалось загрузить библиотеку обработки\n")
                     .append(sLibName));
        return vslOut;
    }

    typedef QVector<QStringList> (*gM)(QVector<QStringList>);
    gM pft = (gM) libProc.resolve("processData");
    if (pft)
    {
        if (QString::compare(sFileName.split(".").last(),"xls") == 0)
        {vslRaw = this->getXlsData(sFileName);}
        if (QString::compare(sFileName.split(".").last(),"xlsx") == 0)
        {vslRaw = this->getXlsxData(sFileName);}
//        qDebug() << rawVector << "\n";
        if (!vslRaw.isEmpty())
        {
            vslOut = pft(vslRaw);
//            qDebug() << outVector;
            if (vslOut.isEmpty())
            {
                // vector processing error here
                this->setStatus(true, "Ошибка обработки");
                return vslOut;
            }
        }
        else
        {
            // error opening excel file here
            this->setStatus(true, "Ошибка чтения файла");
            return vslOut;
        }
    }
    else
    {
        // .so resolve error here here
        this->setStatus(true, QString("Не удалось получить информацию из библиотеки обработки\n")
                        .append(sLibName));
        return vslOut;
    }
    libProc.unload();

    return vslOut;
}

QString files::getServerIp()
{
    QFile fSettings(sServerIpFileName);
    if (fSettings.open(QFile::ReadOnly))
    {
//        const QString &errWS("Путь каталога или значение не должны содержать символа ");
        QString sFline;

        QTextStream tsIn(&fSettings);
        tsIn.setCodec("UTF-8");
        do
        {
            sFline = tsIn.readLine();

            if (sFline.isEmpty() || QString::compare(sFline.at(0),"#") == 0)
            {continue;}

            return sFline;
            continue;
        } while (!sFline.isNull());
    }
    return QString();
}

#include "dlg_settings.h"
#include "ui_dlg_settings.h"

dlg_settings::dlg_settings(QWidget *parent) : QDialog(parent), ui(new Ui::dlg_settings)
{
    ui->setupUi(this);

    modelProcLibs=0;
    ui->tvw_procLibs->setFixedWidth(this->width() * 0.864);
    this->loadSettings();
}

dlg_settings::~dlg_settings()
{
    delete ui;
}


void dlg_settings::loadSettings()
{
    QPointer<files> fw = new files(this);
    connect(fw, SIGNAL(setStatus(bool,QString)), this, SLOT(logger(bool,QString)));
    slSettings = fw->readSettings();
    lslLibInfo = fw->loadProcLibs(slSettings.at(1));
    delete fw;

    ui->le_invoiceDir->setText(slSettings.at(0));
    ui->le_procLibDir->setText(slSettings.at(1));

    if (modelProcLibs!=0) {delete modelProcLibs; modelProcLibs = 0;}
    modelProcLibs = new QStandardItemModel(this);
    for (int irow=0; irow<lslLibInfo.size(); irow++)
    {
        QStandardItem *siVendor = new QStandardItem(lslLibInfo.at(irow).at(0));
        QStandardItem *siVersion = new QStandardItem(lslLibInfo.at(irow).at(1));
        QStandardItem *siLpath = new QStandardItem(lslLibInfo.at(irow).at(2));

        QList<QStandardItem *> lsiModelRow;
        lsiModelRow.append(siVendor);
        lsiModelRow.append(siVersion);
        lsiModelRow.append(siLpath);
        modelProcLibs->appendRow(lsiModelRow);
    }
    modelProcLibs->setHorizontalHeaderLabels(QStringList() << "Поставщик" << "Версия" << "Путь");

    ui->tvw_procLibs->setModel(modelProcLibs);
    ui->tvw_procLibs->setTextElideMode(Qt::ElideLeft);
    int effWidth = ui->tvw_procLibs->width();
    ui->tvw_procLibs->setColumnWidth(0, effWidth * 0.18);
    ui->tvw_procLibs->setColumnWidth(1, effWidth * 0.14);
    ui->tvw_procLibs->setColumnWidth(2, effWidth * 0.6);
}

void dlg_settings::saveSettings()
{
    QStringList slOut;
    slOut << slSettings.at(0) << slSettings.at(1) << "ip;;ipp-wh-01";

    QPointer<files> fw = new files(this);
    connect(fw, SIGNAL(setStatus(bool,QString)), this, SLOT(logger(bool,QString)));
    fw->writeSettings(slSettings);
    delete fw;
}

void dlg_settings::logger(const bool &stType, const QString &sError)
{
    if (stType==1)
    {QMessageBox::critical(this,"Ошибка",sError,QMessageBox::Ok);}
}

void dlg_settings::on_btn_invOpen_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + slSettings.at(0)));
}

void dlg_settings::on_btn_invChange_clicked()
{
    slSettings[0] = QFileDialog::getExistingDirectory(this, "Укажите директорию с накладными",
                        slSettings.at(0), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    this->saveSettings();
    this->loadSettings();
}

void dlg_settings::on_btn_plOpen_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + slSettings.at(1)));
}

void dlg_settings::on_btn_plChange_clicked()
{
    slSettings[1] = QFileDialog::getExistingDirectory(this, "Укажите директорию с библиотеками обработки",
                        slSettings.at(1), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    this->saveSettings();
    this->loadSettings();
}

void dlg_settings::on_btn_add_clicked()
{
    QString sFileName = QFileDialog::getOpenFileName(this, "Выберите файл библиотеки", slSettings.at(1),
                                                    QObject::tr("Файлы библиотек (*.so)"));
    QPointer<files> fw = new files(this);
    connect(fw, SIGNAL(setStatus(bool,QString)), this, SLOT(logger(bool,QString)));
    QStringList sLibInfo = fw->getProcLibInfo(sFileName);
    delete fw;

    if (sLibInfo.at(0).isEmpty() && sLibInfo.at(1).isEmpty())
    {
        this->logger(1, QString("Не удалось получить информацию из библиотеки обработки\n")
                        .append(sLibInfo.at(2)));
        return;
    }

    QString sNewName = QString(slSettings.at(1)).append('/').append(sFileName.split('/').last());
    if(QString::compare(sFileName,sNewName)!=0)
    {
        if (QFile::exists(sNewName))
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Внимание", QString("Библиотека ").append(sFileName.split('/').last())
                                          .append(" существует в каталоге библиотек\nЗаменить?"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
            {this->loadSettings();return;}
        }
        QFile::remove(sNewName);
        QFile::copy(sFileName,sNewName);
        QFile::remove(sFileName);
    }
    else
    {this->logger(1,"Файл уже существует в этом каталоге");}

    this->loadSettings();
}

void dlg_settings::on_btn_delete_clicked()
{
    QModelIndex midx = ui->tvw_procLibs->currentIndex();

    if (!midx.isValid())
    {return;}

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Удаление", QString("Библиотека ").append(lslLibInfo.at(midx.row()).at(0))
                                  .append(" будет навсегда удалена\nВы хотите продолжить?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No)
    {return;}

    QFile::remove(lslLibInfo.at(midx.row()).at(2));

    this->loadSettings();
}

void dlg_settings::on_btn_refresh_clicked()
{
    this->loadSettings();
}

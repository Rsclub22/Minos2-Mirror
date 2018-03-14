#include "base_pch.h"

#include <QFileDialog>

#include "ConfigElementFrame.h"
#include "ui_ConfigElementFrame.h"

ConfigElementFrame::ConfigElementFrame(bool nele) :
    QFrame(0),
    newElement(nele),
    ui(new Ui::ConfigElementFrame)
{
    ui->setupUi(this);

    inhibitIndexChange = true;
    QStringList appTypes = MinosConfig::getMinosConfig()->getAppTypes();
    ui->appTypeCombo->addItems(appTypes);
    ui->appTypeCombo->setCurrentIndex(appTypes.indexOf("None"));
    inhibitIndexChange = false;

    gradient.setColorAt(0.0, Qt::lightGray);
    gradient.setColorAt(1.0, Qt::white);


}

ConfigElementFrame::~ConfigElementFrame()
{
    delete ui;
}
void ConfigElementFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    gradient.setStart(rect().left(), rect().top());
    gradient.setFinalStop(rect().left(), rect().height());
    painter.fillRect(event->rect(), QBrush(gradient));
}
void ConfigElementFrame::setElement(QSharedPointer<RunConfigElement> c)
{
    this->configElement = c;

    localOK = c->localOK;
    remoteOK = c->remoteOK;

    ui->rbRunLocally->setVisible(c->localOK);
    ui->rbConnectRemote->setVisible(c->remoteOK);


    if (localOK && c->runType == RunLocal)
    {
        ui->rbRunLocally->setChecked(true);
    }
    else if (remoteOK && c->runType == ConnectServer)
    {
        ui->rbConnectRemote->setChecked(true);
        c->showAdvanced = true;
    }
    else if (localOK)
    {
        ui->rbRunLocally->setChecked(true);
        c->runType = RunLocal;
    }
    else if (remoteOK)
    {
        ui->rbConnectRemote->setChecked(true);
        c->runType = ConnectServer;
        c->showAdvanced = true;
    }
    else if (!localOK && !remoteOK)
    {
        c->rEnabled = false;
    }


    ui->enabledCheckbox->setChecked(c->rEnabled);

    inhibitIndexChange = true;
    QString at = c->appType;
    ui->appTypeCombo->setCurrentText(at);
    inhibitIndexChange = false;

    if (c->deleted)
    {
        ui->elementNameEdit->setText("<Deleted>");
    }
    else
    {
        ui->elementNameEdit->setText(c->name);
    }
    ui->programNameEdit->setText(c->commandLine);
    ui->homeDirectoryEdit->setText(c->rundir);
    ui->parametersEdit->setText(c->params);
    ui->serverNameEdit->setText(c->server);
    ui->remoteAppNameEdit->setText(c->remoteApp);

    ui->advancedCheckbox->setChecked(c->showAdvanced);
    ui->enabledCheckbox->setChecked(c->rEnabled);
    ui->hideAppCheckBox->setChecked(c->hideApp);

    checkEnabled();
}
void ConfigElementFrame::setNameFocus()
{
    ui->elementNameEdit->setFocus();
}
void ConfigElementFrame::fixComboStyle()
{
   // ui->appTypeCombo->setStyleSheet("");
}

void ConfigElementFrame::saveElement()
{
    if (configElement)
    {
        if (ui->rbRunLocally->isChecked())
            configElement->runType = RunLocal;
        if (ui->rbConnectRemote->isChecked())
            configElement->runType = ConnectServer;

        configElement->showAdvanced = ui->advancedCheckbox->isChecked();
        configElement->rEnabled = ui->enabledCheckbox->isChecked();
        configElement->hideApp = ui->hideAppCheckBox->isChecked();

        configElement->appType = ui->appTypeCombo->currentText();

        QString c = ui->elementNameEdit->text().trimmed();
        if (c.compare("<Deleted>", Qt::CaseInsensitive) == 0)
        {
            configElement->deleted = true;
        }
        else
        {
            configElement->name = c;
        }
        configElement->rundir = ui->homeDirectoryEdit->text().trimmed();
        configElement->commandLine = ui->programNameEdit->text().trimmed();
        configElement->params = ui->parametersEdit->text().trimmed();
        configElement->server = ui->serverNameEdit->text().trimmed();
        configElement->remoteApp = ui->remoteAppNameEdit->text().trimmed();

        configElement->localOK = localOK;
        configElement->remoteOK = remoteOK;

        configElement->newElement = newElement;

        if (newElement)
        {
            MinosConfig::getMinosConfig() ->elelist.append(configElement );
            newElement = false;
        }
    }
}
void ConfigElementFrame::on_programBrowseButton_clicked()
{
    // We need
    QDir cdir(GetCurrentDir());
    QString InitialDir = ExtractFileDir(ui->programNameEdit->text());

    QFileDialog dialog(this, "Minos Component Program", InitialDir);
#if QT_VERSION >= 0x050600
    const QStringList schemes = QStringList(QStringLiteral("file"));

    dialog.setSupportedSchemes(schemes);
#endif

#ifdef Q_OS_WIN
    QString Filter = "Executable Files (*.exe);;"
                     "All Files (*.*)" ;
    dialog.setNameFilter(Filter);
#else
    dialog.setFilter(QDir::AllDirs | QDir::Files | QDir::Dirs /*| QDir::Executable*/); //executable doesn't seem to work
#endif

    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted)
    {
        // need to make path relative
        QString progName = dialog.selectedUrls().value(0).toLocalFile();
        QString rpath = cdir.relativeFilePath(progName);
        ui->programNameEdit->setText(rpath);

    }

}

void ConfigElementFrame::on_homeDirectoryBrowse_clicked()
{
    QDir cdir(GetCurrentDir());

    QString fpath =  ui->homeDirectoryEdit->text();

    QString destDir = QFileDialog::getExistingDirectory(
                  0,
                  "Set Working Directory",
                  fpath,
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                   );
    if (!destDir.isEmpty())
    {
        QString rpath = cdir.relativeFilePath(destDir);
        ui->homeDirectoryEdit->setText(rpath);
    }
}

void ConfigElementFrame::on_deleteButton_clicked()
{
    // mark the element as deleted
    ui->elementNameEdit->setText("<Deleted>");
    ui->enabledCheckbox->setChecked(false);
}

void ConfigElementFrame::checkEnabled()
{
    if (!ui->enabledCheckbox->isChecked())
    {
        ui->programFrame->setEnabled(false);
        ui->serverFrame->setEnabled(false);
        ui->actionsGroup->setEnabled(false);
    }
    else if (ui->rbRunLocally->isChecked())
    {
        ui->programFrame->setEnabled(true);
        ui->serverFrame->setEnabled(false);
        ui->actionsGroup->setEnabled(true);
    }
    else if (ui->rbConnectRemote->isChecked())
    {
        ui->programFrame->setEnabled(false);
        ui->serverFrame->setEnabled(true);
        ui->actionsGroup->setEnabled(true);
    }

    ui->advancedGroup->setVisible( ui->advancedCheckbox->isChecked());
}

void ConfigElementFrame::on_rbRunLocally_clicked()
{
    checkEnabled();
}

void ConfigElementFrame::on_rbConnectRemote_clicked()
{
    checkEnabled();
}

void ConfigElementFrame::on_appTypeCombo_currentIndexChanged(const QString &value)
{
    if (!inhibitIndexChange)
    {
        ui->elementNameEdit->setText(value);

        ui->remoteAppNameEdit->setText(value);

        AppConfigElement ace = MinosConfig::getMinosConfig()->getAppConfigElement(value);
        ui->programNameEdit->setText(ace.appPath);

        if (ui->homeDirectoryEdit->text().isEmpty())
            ui->homeDirectoryEdit->setText(".");

        localOK = ace.localOK;
        remoteOK = ace.remoteOK;
        ui->hideAppCheckBox->setChecked(ace.defaultHide);
        ui->enabledCheckbox->setChecked(true);

        ui->rbRunLocally->setVisible(localOK);
        ui->rbConnectRemote->setVisible(remoteOK);

        if (localOK)
            ui->rbRunLocally->setChecked(true);
        else if (remoteOK)
        {
            ui->rbConnectRemote->setChecked(true);
            ui->advancedCheckbox->setChecked(true);
        }

        saveElement();

        checkEnabled();
    }
}

void ConfigElementFrame::on_advancedCheckbox_clicked()
{
    checkEnabled();
}

void ConfigElementFrame::on_enabledCheckbox_clicked()
{
    checkEnabled();
}

void ConfigElementFrame::on_hideAppCheckBox_clicked()
{
    configElement->hideApp = ui->hideAppCheckBox->isChecked();

    if (configElement->hideApp)
        configElement->sendCommand("HideServers");
    else
        configElement->sendCommand("ShowServers");

}

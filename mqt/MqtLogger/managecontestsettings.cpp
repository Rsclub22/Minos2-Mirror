#include <QSettings>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include "AppStartup.h"
#include "regsettings.h"
#include "contestdetails.h"
#include "MTrace.h"

#include "managecontestsettings.h"
#include "ui_managecontestsettings.h"

QMap<QString, ContestSettings> ManageContestSettings::allSettings;
QString const ManageContestSettings::defaultContestSettings = "default";

ManageContestSettings::ManageContestSettings(ContestDetails *parent, QString cname) :
    QDialog(parent),
    ui(new Ui::ManageContestSettings),
    parentDetails(parent)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    getAllSettings();

    QMap<QString, ContestSettings>::Iterator csi = allSettings.find(cname);
    if (csi != allSettings.end())
    {
        settings = &(*csi);
    }
    if (!cname.isEmpty())
    {
        if (!settings)
        {
            settings = &allSettings[cname];
        }
        parentDetails->toSettings(settings);
    }

    RegSettings rsettings;
    QByteArray geometry = rsettings.getSettings().value(geoString + "/geometry/").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    RegSettings spsettings;
    QString key = geoString + "/SplitterState/";
    QByteArray state = spsettings.getSettings().value(key).toByteArray();
    ui->settingsSplitter->restoreState(state);

    ui->SettingsList->setMinimumWidth(10);
    ui->OptionsTable->setMinimumWidth(10);
}
ManageContestSettings::~ManageContestSettings()
{
    delete ui;
}
ContestSettings *ManageContestSettings::getCurrentSettings(QString &cname)
{
    ContestSettings *settings = nullptr;
    if (!cname.isEmpty())
    {
        QMap<QString, ContestSettings>::Iterator csi = allSettings.find(cname);
        if (csi != allSettings.end())
        {
            settings = &(*csi);
        }
        else
        {
            QMap<QString, ContestSettings>::Iterator csi =
                allSettings.find(defaultContestSettings);
            if (csi != allSettings.end())
            {
                settings = &(*csi);
            }
        }
    }
    return settings;
}
void ManageContestSettings::getSettings(QString cname,
                                        ContestDetails *parentDetails)
{
    getAllSettings();

    ContestSettings *settings = getCurrentSettings(cname);
    if (settings)
    {
        parentDetails->fromSettings(settings);
    }
}
void ManageContestSettings::getAllSettings()
{
    // read in settings JSON
    QString f = getDirectoryLocation(dlConfiguration) + "/ContestSettings.json";
    QFile jf(f);
    if (jf.open(QIODevice::ReadOnly))
    {
        QString s = jf.readAll();
        QJsonParseError err;
        QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
        if (!err.error && json.isObject())
        {
            QJsonObject ac = json.object();
            for (QJsonObject::const_iterator cf = ac.begin(); cf != ac.end(); cf++)
            {
                ContestSettings cs;

                QString a = cf.key();
                QJsonObject v = cf.value().toObject();

                cs.contestName = a;
                cs.stationBundle = v.value( "stationBundle").toString();

                cs.entryBundle = v.value( "entryBundle").toString();
                cs.QTHBundle = v.value( "QTHBundle").toString();

                cs.section = v.value( "section").toString();

                cs.radio = v.value( "radio").toString();
                cs.rotator = v.value( "rotator").toString();

                cs.mainOp = v.value( "mainOp").toString();
                cs.secondOp = v.value( "secondOp").toString();

                cs.screenLayout = v.value( "screenLayout").toString();

                cs.logSet = v.value( "logSet").toString();
                cs.appSet = v.value( "appSet").toString();

                allSettings[a] = cs;
            }
        }
    }
}

void ManageContestSettings::saveAllSettings()
{
    //save everything back to json
    QJsonDocument json;

    QJsonObject sets;

    for (QMap<QString, ContestSettings>::Iterator csi  = allSettings.begin(); csi != allSettings.end(); csi++)
    {
        QJsonObject set;
        set.insert("stationBundle", csi->stationBundle);
        set.insert("entryBundle", csi->entryBundle);
        set.insert("QTHBundle", csi->QTHBundle);

        set.insert("section", csi->section);

        set.insert("radio", csi->radio.toString());
        set.insert("rotator", csi->rotator.toString());

        set.insert("mainOp", csi->mainOp);
        set.insert("secondOp", csi->secondOp);

        set.insert("screenLayout", csi->screenLayout);

        set.insert("logSet", csi->logSet);
        set.insert("appSet", csi->appSet);


        sets.insert(csi.key(), set);
    }
    json.setObject(sets);

    QByteArray s = json.toJson(QJsonDocument::Indented);

    QString f = getDirectoryLocation(dlConfiguration) + "/ContestSettings.json";
    QFile jf(f);
    if (!jf.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        trace("Failed to open " +  f);
        return;
    }
    jf.write(s);

    jf.close();
}

void ManageContestSettings::on_settingsSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings rsettings;
    QByteArray state = ui->settingsSplitter->saveState();
    QString key = geoString + "/SplitterState/";
    rsettings.getSettings().setValue(key, state);
}
int ManageContestSettings::exec()
{
    showSettings();
    showDetails();

    return QDialog::exec();
}

void ManageContestSettings::showSettings()
{
    ui->SettingsList->clear();
    QStringList s = allSettings.keys();

    int offset = 0;

    for ( int i = 0; i < s.size(); i++ )
    {
        auto ai = allSettings.find(s[i]);
        if (ai != allSettings.end())
        {
            if (settings && &(*ai) == settings)
            {
                offset = i;
            }
        }
        ui->SettingsList->addItem( s[ i ] );
    }
    if (offset < s.count())
    {
        settings = &allSettings[s[offset]];
    }

    ui->SettingsList->setCurrentRow(offset);
    showSetting( );
}
//---------------------------------------------------------------------------
void ManageContestSettings::showSetting()
{
    ui->OptionsTable->setVisible(true);
 }
void ManageContestSettings::showDetails()
{
    ui->OptionsTable->clear();

    if (settings)
    {
        QStringList labels = settings->getHeaders();
        QStringList values = settings->getValues();

        ui->OptionsTable->setColumnCount(1);
        ui->OptionsTable->setRowCount(labels.size());

        for ( int i= 0; i < labels.size(); i++ )
        {

            QTableWidgetItem *it = new QTableWidgetItem(values[i]);
            Qt::ItemFlags fl = it->flags() | Qt::ItemIsEditable;
            it->setFlags(fl);
            ui->OptionsTable->setItem(i, 0, it);
        }
        ui->OptionsTable->setVerticalHeaderLabels(labels);

        // something messes with the splitter settings, so reset them
        RegSettings spsettings;
        QString key = geoString + "/SplitterState/";
        QByteArray state = spsettings.getSettings().value(key).toByteArray();
        ui->settingsSplitter->restoreState(state);
    }
}
void ManageContestSettings::getDetails()
{
    if (ui->OptionsTable->rowCount())
    {
        for ( int r = 0; r < ecsMaxVal; r++ )
        {
            QTableWidgetItem *qtwi = ui->OptionsTable->item(r, 0);
            if (qtwi)
            {
                QString val = qtwi->text();
                settings->setVal(static_cast<eCSettings>(r), val);
                switch(r)
                {
                default:
                    break;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------

void ManageContestSettings::on_CancelButton_clicked()
{
    reject();
}

void ManageContestSettings::on_OKButton_clicked()
{
    getDetails();
    saveAllSettings();
    accept();
}
void ManageContestSettings::on_deleteButton_clicked()
{
    // delete the currently selected setting
    if (ui->SettingsList->count())
    {
        QString val = ui->SettingsList->currentItem()->text();
        int removed = allSettings.remove(val);
        if (removed > 0)
        {
            settings = nullptr;
            showSettings();
            showDetails();
        }
    }
}
void ManageContestSettings::on_SettingsList_itemSelectionChanged()
{
    if (settings)
    {
        getDetails();  // save what is set already

        // and we need to change "settings"

        QString val = ui->SettingsList->currentItem()->text();
        settings = &allSettings[val];
        showSetting();
    }
}

void ManageContestSettings::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue(geoString + "/geometry/" , saveGeometry());
}
void ManageContestSettings::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void ManageContestSettings::accept()
{
    doCloseEvent();
    getDetails();
    parentDetails->fromSettings(settings);
    QDialog::accept();
}


bool ContestSettings::operator <(const ContestSettings &rhs) const
{
    return contestName < rhs.contestName;
}

QString ContestSettings::getVal(eCSettings s) const
{
    QString val;
    switch(s)
    {
    case ecsStation:
        val = stationBundle;
        break;
    case ecsEntry:
        val = entryBundle;
        break;
    case ecsQTH:
        val = QTHBundle;
        break;
    case ecsSection:
        val = section;
        break;
    case ecsRadio:
        val = radio.toString();
        break;
    case ecsRotator:
        val = rotator.toString();
        break;
    case ecsMainOp:
        val = mainOp;
        break;
    case ecsSecondOp:
        val = secondOp;
        break;
    case ecsScreenLayout:
        val = screenLayout;
        break;
    case ecsLogSet:
        val = logSet;
        break;
    case ecsStartApps:
        val = appSet;
        break;
    case ecsMaxVal:
        break;
    }
    return val;
}

void ContestSettings::setVal(eCSettings s, QString val)
{
    switch(s)
    {
    case ecsStation:
        stationBundle = val;
        break;
    case ecsEntry:
        entryBundle = val;
        break;
    case ecsQTH:
        QTHBundle = val;
        break;
    case ecsSection:
        section = val;
        break;
    case ecsRadio:
        radio = PubSubName(val);
        break;
    case ecsRotator:
        rotator = PubSubName(val);
        break;
    case ecsMainOp:
        mainOp = val;
        break;
    case ecsSecondOp:
        secondOp = val;
        break;
    case ecsScreenLayout:
        screenLayout = val;
        break;
    case ecsLogSet:
        logSet = val;
        break;
    case ecsStartApps:
        appSet = val;
        break;
    case ecsMaxVal:
        break;
    }

}
QString ContestSettings::headerName(eCSettings s) const
{
    QString val;
    switch(s)
    {
    case ecsStation:
        val = tr("Station Settings");
        break;
    case ecsEntry:
        val = tr("Entry Settings");
        break;
    case ecsQTH:
        val = tr("QTH Settings");
        break;
    case ecsSection:
        val = tr("Section");
        break;
    case ecsRadio:
        val = tr("Radio");
        break;
    case ecsRotator:
        val = tr("Rotator");
        break;
    case ecsMainOp:
        val = tr("Main Op");
        break;
    case ecsSecondOp:
        val = tr("Second Op");
        break;
    case ecsScreenLayout:
        val = tr("Screen Layout");
        break;
    case ecsLogSet:
        val = tr("Contest Log Set");
        break;
    case ecsStartApps:
        val = tr("Start Applications");
        break;
    case ecsMaxVal:
        break;
    }

    return val;
}
QStringList ContestSettings::getHeaders() const
{
    QStringList h;
    for (int i = 0; i < ecsMaxVal; i++)
    {
        h.append(headerName(static_cast<eCSettings>(i)));
    }
    return h;
}

QStringList ContestSettings::getValues() const
{
    QStringList h;
    for (int i = 0; i < ecsMaxVal; i++)
    {
        h.append(getVal(static_cast<eCSettings>(i)));
    }
    return h;
}

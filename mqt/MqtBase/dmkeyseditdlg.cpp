#include <QSettings>
#include <QString>
#include <QPushButton>
#include <QToolButton>

#include <QCheckBox>
#include <QLineEdit>

#include "regsettings.h"
#include "enqdlg.h"
#include "MShowMessageDlg.h"
#include "MMessageDialog.h"
#include "dmkeyseditdlg.h"
#include "dmbuttoneditaddradiodialog.h"
#include "ui_dmkeyseditdlg.h"
#include "txkeyerCommonConstants.h"
//#include "txVmExternalButtonDialog.h"
#include "txvminternalbuttondialog.h"
#include "txkeyerCommonConstants.h"
#include <QDebug>

using namespace TxKeyerCommon;

DMKeysEditDlg::DMKeysEditDlg(QWidget *parent, QString fKeyFileName, QString name, KeyerMap &allKeyConfigs, QString txKeyerType, QString rigName, const QStringList listOfRadios) :
    QDialog(parent),
    ui(new Ui::DMKeysEditDlg),
    allKeyConfigs(allKeyConfigs),
    name(name),
    rigName(rigName),
    txKeyerType(txKeyerType),
    listOfRadios(listOfRadios)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("DMKeysEdit/geometry/" + name).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state = settings.getSettings().value("DMKeysEdit/SplitterState/" + name).toByteArray();
    ui->settingsSplitter->restoreState(state);

    QString baseTitle = windowTitle();
    setWindowTitle(baseTitle + " - " + fKeyFileName) ;
/*
    ui->NewSectionButton->setText(tr("New Digi FKey section"));
    ui->CopyButton->setText(tr("Copy Digi FKey section"));
    ui->DeleteButton->setText(tr("Delete Digi FKey section"));
    ui->renameButton->setText(tr("Rename Digi FKey section"));
*/
    ui->NewSectionButton->setText(tr("New Macro FKey section"));
    ui->CopyButton->setText(tr("Copy Macro FKey section"));
    ui->DeleteButton->setText(tr("Delete Macro FKey section"));
    ui->renameButton->setText(tr("Rename Macro FKey section"));

    showRadioListButtons(false);

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        RadioList = new QListWidget(this);
        ui->settingsSplitter->insertWidget(1, RadioList);
        RadioList->setMinimumWidth(10);
        showRadioListButtons(true);
        connect(RadioList, &QListWidget::currentItemChanged, this, &DMKeysEditDlg::onRadioListItemSelectionChanged);
    }


    ui->SectionsList->setMinimumWidth(10);
    ui->OptionsTable->setMinimumWidth(10);

    ui->OptionsTable->horizontalHeader()->setVisible(true);

    connect(ui->OptionsTable->selectionModel(),&QItemSelectionModel::selectionChanged,
            this, &DMKeysEditDlg::onOptionsTableSelectionChanged);

    ui->upButton->setEnabled(false);
    ui->downButton->setEnabled(false);

    QString keySubs =
        tr("* My callsign\n\
        # Serial to send\n\
        ! Their call\n\
        {macro}\n\
        {MYCALL}        my full call\n\
        {CALL}          their full call\n\
        {SN}            serial number to send\n\
        {EXCH}          full exchange to send - can be in multiple parts\n\
        {GRID}          my Maidenhead locator\n\
        {SPACE}         space character\n\
        {SENTRST}       RST to send\n\
        {TIME2}         time in HHmm format\n\
        {LOG}           log the QSO. clear it, and focus on CALL\n\
        {WIPE}          wipe all QSO fields\n\
        {CALLFIELD}     move focus to the call field\n\
        {SERIALFIELD}   move focus to the serial received field\n\
        {EXCHANGEFIELD} move focus to the exchange received field");
    ui->keySubs->setText(keySubs);
}
void DMKeysEditDlg::on_settingsSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->settingsSplitter->saveState();
    settings.getSettings().setValue("DMKeysEdit/SplitterState/" + name, state);
}

int DMKeysEditDlg::exec()
{
    showSections();
    showDetails();

    return QDialog::exec();
}

DMKeysEditDlg::~DMKeysEditDlg()
{
    delete ui;
}

bool sectionLessThan(const QString &s1, const QString &s2)
{
    return s1.toLower() < s2.toLower();
}

void DMKeysEditDlg::showSections()
{
    ui->SectionsList->clear();

    // Extract all contest names for the current keyer type
    QStringList sections;

    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend())
        return;  // No contests for this keyerType

    sections = keyerIt->keys();  // contest names for this keyerType

    std::sort(sections.begin(), sections.end(),
              [](const QString &first, const QString &second) {
                  return sectionLessThan(first, second);
              });

    int offset = 0;
    for (int i = 0; i < sections.size(); ++i)
    {
        if (sections[i] == name)
            offset = i;

        ui->SectionsList->addItem(sections[i]);
    }

    ui->SectionsList->setCurrentRow(offset);

    if (RadioList)  // we have created a radio section
    {
        showRadiosForSection();
    }

    showSection();  // Or showDetails(), if that’s what you want immediately
}

void DMKeysEditDlg::showRadiosForSection()
{
    if (RadioList)
    {
        RadioList->clear();  // Assuming you have a QListWidget* called RadiosList

        const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
        if (keyerIt == allKeyConfigs.cend())
            return;

        const auto contestIt = keyerIt->constFind(name);  // name = selected section/contest
        if (contestIt == keyerIt->cend())
            return;

        // Extract rig names for this contest
        QStringList radioNames = contestIt->keys();

        std::sort(radioNames.begin(), radioNames.end(),
                  [](const QString &a, const QString &b) {
                      return a.compare(b, Qt::CaseInsensitive) < 0;
                  });

        for (const QString &radio : radioNames)
        {
            RadioList->addItem(radio);
        }

        // Optional: auto-select first item or previously used radio
        if (!radioNames.isEmpty())
            RadioList->setCurrentRow(0);
    }


}


//---------------------------------------------------------------------------
void DMKeysEditDlg::showSection()
{
    const int offset = ui->SectionsList->currentRow();

    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend()) {
        ui->OptionsTable->setVisible(false);
        return;
    }

    QStringList contestList = keyerIt->keys();  // <-- FIXED
    std::sort(contestList.begin(), contestList.end(),
              [](const QString &first, const QString &second) {
                  return sectionLessThan(first, second);
              });

    if (offset >= 0 && offset < contestList.size())
    {
        // Update selected contest
        name = contestList[offset];

        ui->OptionsTable->setVisible(true);
        showDetails();
    }
    else
    {
        ui->OptionsTable->setVisible(false);
    }

    //ui->NewSectionButton->setEnabled(true);
    //ui->DeleteButton->setEnabled(offset > 0);
    //ui->CopyButton->setEnabled(offset > 0);
    //ui->renameButton->setEnabled(offset > 0);

    bool isProtected = (name == "<None>" || name == "Default");

    ui->DeleteButton->setEnabled(!isProtected);
    ui->CopyButton->setEnabled(!isProtected);
    ui->renameButton->setEnabled(!isProtected);

}




void DMKeysEditDlg::showDetails()
{
    ui->OptionsTable->clear();

    // ---------- 1. locate the ContestSection ----------
    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend()) return;

    const auto contestIt = keyerIt->constFind(name);
    if (contestIt == keyerIt->cend()) return;

    const auto &rigMap = contestIt.value();

    QString rigKey = getRigKey();

    const ContestSection &sect = rigMap[rigKey];
    const KeySet &run = sect.run;
    const KeySet &sp  = sect.sp;

    if (run.size() != 12 || sp.size() != 12)
        return; // ensure correct data size

    // ---------- 2. build the table ----------
    constexpr int totalColumns = 6;
    ui->OptionsTable->setColumnCount(totalColumns);
    ui->OptionsTable->setRowCount(24);

    QStringList vHeaders;

    for (int row = 0; row < 24; ++row)
    {
        const KeyVal &k = (row < 12 ? run[row] : sp[row - 12]);

        vHeaders << QString("%1 F%2")
                        .arg(row < 12 ? tr("Run") : tr("S&P"))
                        .arg(row < 12 ? row + 1 : row - 11);

        ui->OptionsTable->setItem(row, EDIT_DLG_COL0, new QTableWidgetItem(k.ktop()));
        ui->OptionsTable->setItem(row, EDIT_DLG_COL1, new QTableWidgetItem(k.kval()));
        ui->OptionsTable->setItem(row, EDIT_DLG_COL2, new QTableWidgetItem(QString::number(k.rigVoiceMemNum())));

        auto *cb = new QCheckBox;
        cb->setChecked(k.rptEnable());
        auto *wrapCB = new QWidget;
        auto *layCB  = new QHBoxLayout(wrapCB);
        layCB->addWidget(cb);
        layCB->setAlignment(Qt::AlignCenter);
        layCB->setContentsMargins(0,0,0,0);
        ui->OptionsTable->setCellWidget(row, EDIT_DLG_COL3, wrapCB);

        ui->OptionsTable->setItem(row, EDIT_DLG_COL4, new QTableWidgetItem(QString::number(k.rptDur())));

        auto *recBtn = new QToolButton;
        recBtn->setText("🎙");
        recBtn->setToolTip(tr("Record/Play audio for this message"));
        recBtn->setFixedSize(24, 24);
        recBtn->setProperty("row", row);
        recBtn->setProperty("scope", row < 12 ? 0 : 1); // 0 = Run, 1 = S&P

        auto *wrapBtn = new QWidget;
        auto *layBtn  = new QHBoxLayout(wrapBtn);
        layBtn->addWidget(recBtn);
        layBtn->setAlignment(Qt::AlignCenter);
        layBtn->setContentsMargins(0,0,0,0);
        ui->OptionsTable->setCellWidget(row, EDIT_DLG_COL5, wrapBtn);

        connect(recBtn, &QToolButton::clicked, this, [this, recBtn]() {
            int row = recBtn->property("row").toInt();
            int scope = recBtn->property("scope").toInt(); // 0 = Run, 1 = S&P

            if (txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
            {
                TxVmInternalButtonDialog dlg(this);
                dlg.exec();
            }
            else if (txKeyerType == keyerTypes[TxKeyerId::ExternalVoiceKeyer])
            {
                // handle external keyer
            }
        });
    }

    ui->OptionsTable->setVerticalHeaderLabels(vHeaders);
    ui->OptionsTable->setHorizontalHeaderLabels(
        { tr("Key Top"), tr("Value"), tr("Rig\nMem"),
         tr("Repeat"), tr("Repeat\nDur"), tr("Rec.") });

    // ---------- Column sizing ----------
    auto *hh = ui->OptionsTable->horizontalHeader();
    hh->setStretchLastSection(false);
    hh->setSectionResizeMode(EDIT_DLG_COL0, QHeaderView::Interactive);
    hh->setSectionResizeMode(EDIT_DLG_COL1, QHeaderView::Interactive);
    hh->setSectionResizeMode(EDIT_DLG_COL2, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL3, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL4, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL5, QHeaderView::Fixed);

    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL0, 140);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL1, 220);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL2, 50);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL3, 50);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL4, 60);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL5, 30);

    if (txKeyerType == keyerTypes[TxKeyerId::DigitalModes])
    {
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL2, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL3, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL4, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL5, true);
    }

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL1, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL5, true);
    }

    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl] || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL2, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL5, true);
    }

    // ---------- Restore splitter position ----------
    RegSettings settings;
    ui->settingsSplitter->restoreState(
        settings.getSettings().value("DMKeysEdit/SplitterState/" + name).toByteArray());
}



void DMKeysEditDlg::getDetails()
{
    if (!ui->OptionsTable->rowCount())
        return;

    auto &contestMap = allKeyConfigs[txKeyerType][name];
    qDebug() << "getDetails name = " << name;
    // Locate rig key (use rigName, fallback if needed)
    QString rigKey = getRigKey();


    ContestSection &section = contestMap[rigKey];

    if (section.run.isEmpty() && section.sp.isEmpty())
        return;

    int runCount = section.run.size();
    int totalRows = ui->OptionsTable->rowCount();

    for (int r = 0; r < totalRows; ++r)
    {
        KeyVal *entry = nullptr;
        if (r < runCount)
            entry = &section.run[r];
        else if (r - runCount < section.sp.size())
            entry = &section.sp[r - runCount];
        else
            continue;

        // Column 0: Key Top
        if (QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL0))
            entry->setKtop(qtwi->text());

        // Column 1: Value
        if (QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL1))
            entry->setKval(qtwi->text());

        // Column 2: Rig Voice Mem Num
        if (QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL2))
        {
            bool ok;
            int mem = qtwi->text().toInt(&ok);
            if (ok)
                entry->setRigVoiceMemNum(mem);
        }

        // Column 3: Repeat Enable (checkbox)
        if (QWidget *wrap = ui->OptionsTable->cellWidget(r, EDIT_DLG_COL3))
        {
            if (QCheckBox *cb = wrap->findChild<QCheckBox*>())
                entry->setRptEnable(cb->isChecked());
        }

        // Column 4: Repeat Duration
        if (QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL4))
        {
            bool ok;
            int dur = qtwi->text().toInt(&ok);
            if (ok)
                entry->setRptDur(dur);
        }
    }
}


void DMKeysEditDlg::saveCurrentSection()
{
    if (!isCurrentSectionDirty())
        return;

    getDetails();  // Store current UI into model
    clearDirtyFlag();  // Reset dirty status after save
}

bool DMKeysEditDlg::isCurrentSectionDirty() const
{
    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend()) return false;

    const auto contestIt = keyerIt->constFind(name);
    if (contestIt == keyerIt->cend()) return false;

    QString rigKey = getRigKey();

    const auto &section = contestIt->value(rigKey);

    for (const auto &k : section.run)
        if (k.isDirty()) return true;
    for (const auto &k : section.sp)
        if (k.isDirty()) return true;

    return false;
}

void DMKeysEditDlg::clearDirtyFlag()
{
    auto &section = allKeyConfigs[txKeyerType][name][rigName.isEmpty() ? KEYER_NO_RADIO : rigName];
    for (auto &k : section.run)
        k.clearDirty();
    for (auto &k : section.sp)
        k.clearDirty();
}





//---------------------------------------------------------------------------

void DMKeysEditDlg::on_NewSectionButton_clicked()
{
    //getDetails();  // Save current table to model
    saveCurrentSection();

    QString newName = "new digi key section";

    if (enquireDialog(this, tr("Please give a new name for the %1").arg(name), newName))
    {
        auto &contestMap = allKeyConfigs[txKeyerType];

        if (!contestMap.contains(newName))
        {
            // Create empty Run and S&P sets
            ContestSection section;

            for (int i = 1; i <= 12; ++i)
            {
                QString fk = QString("F%1").arg(i);
                KeyVal kv;
                kv.setFk(fk);
                kv.setKtop("");
                kv.setKval("");
                kv.setRigVoiceMemNum(0);
                kv.setRptEnable(false);
                kv.setRptDur(0);

                section.run.append(kv);
            }
            for (int i = 1; i <= 12; ++i)
            {
                QString fk = QString("F%1").arg(i);
                KeyVal kv;
                kv.setFk(fk);
                kv.setKtop("");
                kv.setKval("");
                kv.setRigVoiceMemNum(0);
                kv.setRptEnable(false);
                kv.setRptDur(0);

                section.sp.append(kv);
            }

            // Store under the current rigName (or "noRadio" fallback)
            QString rigKey = getRigKey();

            contestMap[newName][rigKey] = section;


            // Update current selection and UI
            ignoreSectionChange = true;
            name = newName;
            showSections();
            showDetails();
            ignoreSectionChange = false;

        }
        else
        {
            mShowMessage(tr("%1 already exists").arg(newName), this);
        }
    }
}

QString DMKeysEditDlg::getRigKey() const
{
    if (txKeyerType == keyerTypes[TxKeyerId::RigControl] && rigName != "/")
    {
        return rigName;
    }

    return KEYER_NO_RADIO;
}


void DMKeysEditDlg::on_CopyButton_clicked()
{
    //getDetails();  // Save current edits
    saveCurrentSection();

    //int offset = ui->SectionsList->currentRow();
    //if (offset <= 0)
    //    return;



    QString newName = "new digi key section";
    if (!enquireDialog(this, tr("Please give a name for the new %1").arg(name), newName))
        return;

    auto &contestMap = allKeyConfigs[txKeyerType];

    if (contestMap.contains(newName))
    {
        mShowMessage(tr("%1 already exists").arg(newName), this);
        return;
    }

    QString rigKey = getRigKey();

    // Check source exists
    if (!contestMap.contains(name) || !contestMap[name].contains(rigKey))
    {
        mShowMessage(tr("Cannot copy — source section missing."), this);
        return;
    }

    // Deep copy section
    contestMap[newName][rigKey] = contestMap[name][rigKey];

    // Update UI
    name = newName;
    ignoreSectionChange = true;
    showSections();
    showDetails();
    ignoreSectionChange = false;
}


void DMKeysEditDlg::on_DeleteButton_clicked()
{
    //int offset = ui->SectionsList->currentRow();
    //if (offset <= 0) {
    //    mShowMessage(tr("You cannot delete the empty %1!").arg(name), this);
    //    return;
    //}

    if (name == "<None>"  || name == "Default")
    {
        mShowMessage(tr("You cannot rename the empty %1!").arg(name), this);
        return;
    }

    if (!mShowYesNoMessage(this, tr("Are you sure you want to delete the current %1?").arg(name)))
        return;

    QString rigKey = getRigKey();

    auto keyerIt = allKeyConfigs.find(txKeyerType);
    if (keyerIt == allKeyConfigs.end())
        return;

    auto &contestMap = keyerIt.value();
    auto contestIt = contestMap.find(name);
    if (contestIt == contestMap.end())
        return;

    contestIt->remove(rigKey);  // remove this rig's section
    if (contestIt->isEmpty())
        contestMap.remove(name);  // if no rigs left, remove contest

    if (contestMap.isEmpty())
        allKeyConfigs.remove(txKeyerType);  // optional cleanup

    // Reset selection to <None> or first available
    if (!contestMap.isEmpty())
        name = contestMap.firstKey();
    else
        name = QStringLiteral("<None>");

    ignoreSectionChange = true;
    showSections();
    showDetails();
    ignoreSectionChange = false;
}



void DMKeysEditDlg::on_renameButton_clicked()
{
    //getDetails();  // Save current edits
    saveCurrentSection();

    //int offset = ui->SectionsList->currentRow();
    //if (offset == 0)
    //{
    //    mShowMessage(tr("You cannot rename the empty %1!").arg(name), this);
    //    return;
    //}

    if (name == "<None>"  || name == "Default")
    {
        mShowMessage(tr("You cannot rename the empty %1!").arg(name), this);
        return;
    }

    QString newName = name;
    if (enquireDialog(this, tr("Please give a new name for the %1").arg(name), newName))
    {
        auto &contestMap = allKeyConfigs[txKeyerType];

        if (!contestMap.contains(newName))
        {
            // Rename by moving value and removing old key
            contestMap[newName] = contestMap[name];
            contestMap.remove(name);

            name = newName;

            ignoreSectionChange = true;
            showSections();
            showDetails();
            ignoreSectionChange = false;
        }
        else
        {
            mShowMessage(tr("%1 already exists").arg(newName), this);
        }
    }
}

void DMKeysEditDlg::on_addRadioButton_clicked()
{
    if (name == "<None>"  || name == "Default")
    {
        mShowMessage(tr("You cannot add radio to Default!"), this);
        return;
    }


    DmButtonEditAddRadioDialog* addRadioDialog = new DmButtonEditAddRadioDialog(listOfRadios);
    addRadioDialog->setWindowTitle(tr("Add Radio"));

    int result = addRadioDialog->exec();

    if (result == QDialog::Accepted)
    {
        bool radioExists = false;
        if (checkRadioExists(addRadioDialog->getRadioName(), radioExists))
        {
            if (radioExists)
            {
               mShowMessage(tr("Radio already exists, please select another radio"), this);

            }
            else
            {
                // add radio to this contest set
                // Create empty Run and S&P sets

                auto &contestMap = allKeyConfigs[txKeyerType];

                ContestSection section;

                for (int i = 1; i <= 12; ++i)
                {
                    QString fk = QString("F%1").arg(i);
                    KeyVal kv;
                    kv.setFk(fk);
                    kv.setKtop("");
                    kv.setKval("");
                    kv.setRigVoiceMemNum(0);
                    kv.setRptEnable(false);
                    kv.setRptDur(0);

                    section.run.append(kv);
                }
                for (int i = 1; i <= 12; ++i)
                {
                    QString fk = QString("F%1").arg(i);
                    KeyVal kv;
                    kv.setFk(fk);
                    kv.setKtop("");
                    kv.setKval("");
                    kv.setRigVoiceMemNum(0);
                    kv.setRptEnable(false);
                    kv.setRptDur(0);

                    section.sp.append(kv);
                }

                contestMap[name][addRadioDialog->getRadioName()] = section;

                // Update current selection and UI
                ignoreSectionChange = true;
                showSections();
                showDetails();
                ignoreSectionChange = false;

            }
        }
    }
    else
    {
        return;
    }
}

void DMKeysEditDlg::on_deleteRadioButton_clicked()
{
    if (rigName == "noRadio")
    {
        mShowMessage(tr("You cannot delete noRadio!"), this);
        return;
    }


    if (!mShowYesNoMessage(this, tr("Are you sure you want to delete the radio %1?").arg(rigName)))
    {

        return;


    }
    else
    {
        auto keyerIt = allKeyConfigs.find(txKeyerType);
        if (keyerIt != allKeyConfigs.end())
        {
            ContestMap &contestMap = keyerIt.value();
            auto contestIt = contestMap.find(name);
            if (contestIt != contestMap.end())
            {
                RigMap &rigMap = contestIt.value();
                if (rigMap.contains(rigName))
                {
                    rigMap.remove(rigName);
                    qDebug() << "Deleted radio:" << rigName;

                    if (!rigMap.isEmpty())
                        rigName = rigMap.firstKey();
                    else
                        rigName = QStringLiteral("<NoRadio>");
                }
            }
        }



        ignoreSectionChange = true;
        showSections();
        showDetails();
        ignoreSectionChange = false;
    }
}


void DMKeysEditDlg::on_CancelButton_clicked()
{
    reject();
}

void DMKeysEditDlg::on_OKButton_clicked()
{
    //getDetails();
    saveCurrentSection();
    accept();
}

void DMKeysEditDlg::on_SectionsList_itemSelectionChanged()
{
    if (ignoreSectionChange)        // when making program changes
        return;


    //getDetails();  // Save current edits
    saveCurrentSection();

    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend() || ui->SectionsList->currentRow() < 0)
        return;

    QStringList sections = keyerIt->keys();

    int offset = ui->SectionsList->currentRow();
    if (offset >= 0 && offset < sections.size())
    {
        name = sections[offset];
        //rigName = KEYER_NO_RADIO;       // set default radio
        //showSection();
    }

    if (RadioList)
    {
        RadioList->clear();

        const auto contestIt = keyerIt->constFind(name);  // name = selected section/contest
        if (contestIt == keyerIt->cend())
            return;

        // Extract rig names for this contest
        QStringList radioNames = contestIt->keys();

        std::sort(radioNames.begin(), radioNames.end(),
                  [](const QString &a, const QString &b) {
                      return a.compare(b, Qt::CaseInsensitive) < 0;
                  });

        for (const QString &radio : radioNames)
        {
            RadioList->addItem(radio);
        }

        for (int i = 0; i < RadioList->count(); i++)
        {
            QListWidgetItem *item = RadioList->item(i);
            if (item->text() == KEYER_NO_RADIO)
            {
                RadioList->setCurrentItem(item);
                item->setSelected(true);
            }
        }
    }

    rigName = KEYER_NO_RADIO;       // set default radio
    showSection();
}



void DMKeysEditDlg::onRadioListItemSelectionChanged()
{
    if (ignoreSectionChange)        // when making program changes
        return;

    if (RadioList)
    {


        saveCurrentSection();

        const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
        if (keyerIt == allKeyConfigs.cend())
            return;

        const auto contestIt = keyerIt->constFind(name);  // name = selected section/contest
        if (contestIt == keyerIt->cend())
            return;

        // Extract rig names for this contest
        QStringList radioNames = contestIt->keys();

        int offset = RadioList->currentRow();
        if (offset >= 0 && offset < radioNames.size())
        {
            rigName =radioNames[offset];
            showSection();
        }



    }



}

void DMKeysEditDlg::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("DMKeysEdit/geometry/" + name, saveGeometry());
}
void DMKeysEditDlg::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void DMKeysEditDlg::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void DMKeysEditDlg::onOptionsTableSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &)
{
    ui->upButton->setEnabled(false);
    ui->downButton->setEnabled(false);

    QModelIndexList mil = ui->OptionsTable->selectionModel()->selectedRows();
    if (mil.count() == 1)
    {
        if (mil[0].row() == 0)
        {
            ui->downButton->setEnabled(true);
        }
        else if (mil[0].row() == ui->OptionsTable->model()->rowCount() -1)
        {
            ui->upButton->setEnabled(true);
        }
        else
        {
            ui->upButton->setEnabled(true);
            ui->downButton->setEnabled(true);
        }
    }
}


void DMKeysEditDlg::on_upButton_clicked()
{
    QModelIndexList mil = ui->OptionsTable->selectionModel()->selectedRows();
    if (mil.count() != 1)
        return;

    int selRow = mil[0].row();
    if (selRow == 0)
        return;

    auto &contestMap = allKeyConfigs[txKeyerType];
    if (!contestMap.contains(name))
        return;

    QString rigKey = rigName.isEmpty() ? KEYER_NO_RADIO : rigName;
    if (!contestMap[name].contains(rigKey))
        return;

    ContestSection &section = contestMap[name][rigKey];

    int runCount = section.run.size();
    int totalCount = runCount + section.sp.size();
    if (selRow >= totalCount)
        return;

    // Helper to swap two KeyVal references
    //auto swapKeyVals = [](KeyVal &a, KeyVal &b) {
    //    a.swapWith(b);
    //};

    if (selRow < runCount && selRow - 1 < runCount)
    {
        // Both in run section
        //swapKeyVals(section.run[selRow], section.run[selRow - 1]);
        section.run[selRow].swapWith(section.run[selRow - 1]);
    }
    else if (selRow >= runCount && selRow - 1 >= runCount)
    {
        // Both in sp section
        int spRow = selRow - runCount;
        int spRowPrev = selRow - 1 - runCount;
        //swapKeyVals(section.sp[spRow], section.sp[spRowPrev]);
        section.sp[spRow].swapWith(section.sp[spRowPrev]);
    }
    else if (selRow == runCount)
    {
        // Moving first sp row up to last run row — swap across boundary
        //swapKeyVals(section.sp[0], section.run[runCount - 1]);
        section.sp[0].swapWith(section.run[runCount - 1]);
    }
    else
    {
        // Trying to swap from run to sp not adjacent? Ignore or handle similarly
        return;
    }

    showDetails();
    ui->OptionsTable->selectRow(selRow - 1);
}



void DMKeysEditDlg::on_downButton_clicked()
{
    QModelIndexList mil = ui->OptionsTable->selectionModel()->selectedRows();
    if (mil.count() != 1)
        return;

    int selRow = mil[0].row();
    int rowCount = ui->OptionsTable->rowCount();
    if (selRow >= rowCount - 1)
        return;

    auto &contestMap = allKeyConfigs[txKeyerType];
    if (!contestMap.contains(name))
        return;

    QString rigKey = rigName.isEmpty() ? KEYER_NO_RADIO : rigName;
    if (!contestMap[name].contains(rigKey))
        return;

    ContestSection &section = contestMap[name][rigKey];

    int runCount = section.run.size();
    int totalCount = runCount + section.sp.size();
    if (selRow >= totalCount - 1)
        return;

    //auto swapKeyVals = [](KeyVal &a, KeyVal &b) {
    //    std::swap(a.fk(), b.fk());
    //    std::swap(a.ktop(), b.ktop());
    //    std::swap(a.kval(), b.kval());
    //    std::swap(a.rigVoiceMemNum(), b.rigVoiceMemNum());
    //    std::swap(a.rptEnable(), b.rptEnable());
    //    std::swap(a.rptDur(), b.rptDur());
    //};

    if (selRow < runCount - 1)
    {
        // Both in run section
        //swapKeyVals(section.run[selRow], section.run[selRow + 1]);
        section.run[selRow].swapWith(section.run[selRow + 1]);
    }
    else if (selRow >= runCount && selRow + 1 >= runCount)
    {
        // Both in sp section
        int spRow = selRow - runCount;
        int spRowNext = selRow + 1 - runCount;
        //swapKeyVals(section.sp[spRow], section.sp[spRowNext]);
        section.sp[spRow].swapWith(section.sp[spRowNext]);
    }
    else if (selRow == runCount - 1)
    {
        // Moving last run row down to first sp row - swap across boundary
        //swapKeyVals(section.run[selRow], section.sp[0]);
        section.run[selRow].swapWith(section.sp[0]);
    }
    else
    {
        // Non-adjacent or invalid cases
        return;
    }

    showDetails();
    ui->OptionsTable->selectRow(selRow + 1);
}



void DMKeysEditDlg::showRadioListButtons(bool show)
{
    ui->addRadioButton->setVisible(show);
    ui->deleteRadioButton->setVisible(show);
}

bool DMKeysEditDlg::checkRadioExists(QString radioName, bool &radioExists)
{

    bool ok = false;

    if (allKeyConfigs.contains(txKeyerType))
    {
        const ContestMap &contestMap = allKeyConfigs.value(txKeyerType);

        ok = true;

        if (contestMap.contains(name))
        {
            const RigMap &rigMap = contestMap.value(name);

            if (rigMap.contains(radioName))
            {
                radioExists = true;
            }
            else
            {
                radioExists = false;
            }
        }
    }

    return ok;
}





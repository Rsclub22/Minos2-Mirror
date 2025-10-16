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

DMKeysEditDlg::DMKeysEditDlg(QWidget *parent, QString fKeyFileName, QString minosSelectedContestName_, KeyerMap &allKeyConfigs, QString txKeyerType, PubSubName minosSelectedRadio_, const QStringList listOfRadios) :
    QDialog(parent),
    ui(new Ui::DMKeysEditDlg),
    allKeyConfigs(allKeyConfigs),
    minosSelectedContestName(minosSelectedContestName_),
    txKeyerType(txKeyerType),
    minosSelectedRadio(minosSelectedRadio_),
    listOfRadios(listOfRadios)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("DMKeysEdit/geometry/" + txKeyerType).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state = settings.getSettings().value("DMKeysEdit/SplitterState/" + txKeyerType).toByteArray();
    ui->settingsSplitter->restoreState(state);

    QString baseTitle = windowTitle();
    setWindowTitle(baseTitle + " - " + fKeyFileName) ;

    ui->NewSectionButton->setText(tr("New Macro FKey section"));
    ui->CopyButton->setText(tr("Copy Macro FKey section"));
    ui->DeleteButton->setText(tr("Delete Macro FKey section"));
    ui->renameButton->setText(tr("Rename Macro FKey section"));

    showRadioListButtons(false);

    selectedContestName = minosSelectedContestName;

    if (minosSelectedRadio.isEmpty())
    {
      minosSelectedRadioLocalName = KEYER_NO_RADIO;
    }
    else
    {
        minosSelectedRadioLocalName = minosSelectedRadio.key();

    }

    radioListSelectedName = minosSelectedRadioLocalName;


    if (txKeyerType == keyerTypes[TxKeyerId::RigControl] || txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
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
    settings.getSettings().setValue("DMKeysEdit/SplitterState/" + selectedContestName, state);
}

int DMKeysEditDlg::exec()
{

    ignoreSectionChange = true;
    showSections();
    showDetails();
    ignoreSectionChange = false;

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
        if (sections[i] == selectedContestName)
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
        RadioList->clear();

        const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
        if (keyerIt == allKeyConfigs.cend())
            return;

        const auto contestIt = keyerIt->constFind(selectedContestName);  // name = selected section/contest
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


        //{
        //    RadioList->setCurrentRow(0);
        //}

        QList<QListWidgetItem *> matches = RadioList->findItems(radioListSelectedName, Qt::MatchExactly);
        QListWidgetItem *itemToSelect = nullptr;

        if (!matches.isEmpty())
        {
            itemToSelect = matches.first();
            ui->deleteRadioButton->setEnabled(true);

        }
        else
        {
            // radio not in list, default to noRadio
            QList<QListWidgetItem *> defaultMatches = RadioList->findItems(KEYER_NO_RADIO, Qt::MatchExactly);
            if (!defaultMatches.isEmpty())
            {
                itemToSelect = defaultMatches.first();
                radioListSelectedName = KEYER_NO_RADIO;
                ui->deleteRadioButton->setEnabled(false);
            }


        }

        if (itemToSelect)
        {
            RadioList->setCurrentItem(itemToSelect);
            itemToSelect->setSelected(true);
            RadioList->scrollToItem(itemToSelect);
        }




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
        selectedContestName = contestList[offset];

        ui->OptionsTable->setVisible(true);
        showDetails();
    }
    else
    {
        ui->OptionsTable->setVisible(false);
    }

    ui->NewSectionButton->setEnabled(true);

    bool isProtected = (selectedContestName == "<None>" || selectedContestName == "Default" || selectedContestName == minosSelectedContestName);

    ui->DeleteButton->setEnabled(!isProtected);
    //ui->CopyButton->setEnabled(!isProtected);
    ui->renameButton->setEnabled(!isProtected);

}


void DMKeysEditDlg::setupTableRow(int row, KeyVal &k)
{
    // Determine scope and local row index
    int scope = (row < 12 ? 0 : 1);
    int localRow = (row < 12 ? row : row - 12);

    // Vertical header
    QString header = QString("%1 F%2")
                         .arg(scope == 0 ? tr("Run") : tr("S&P"))
                         .arg(localRow + 1);
    ui->OptionsTable->setVerticalHeaderItem(row, new QTableWidgetItem(header));

    // Columns 0,1,2
    ui->OptionsTable->setItem(row, EDIT_DLG_COL0, new QTableWidgetItem(k.ktop()));
    ui->OptionsTable->setItem(row, EDIT_DLG_COL1, new QTableWidgetItem(k.kval()));
    ui->OptionsTable->setItem(row, EDIT_DLG_COL2, new QTableWidgetItem(QString::number(k.rigVoiceMemNum())));

    // Column 3: Duration Enable checkbox
    auto *durEnableCb = new QCheckBox;
    durEnableCb->setChecked(k.rptEnable());
    auto *wrapDurEnableCb = new QWidget;
    auto *layDurEnableCb = new QHBoxLayout(wrapDurEnableCb);
    layDurEnableCb->addWidget(durEnableCb);
    layDurEnableCb->setAlignment(Qt::AlignCenter);
    layDurEnableCb->setContentsMargins(0,0,0,0);
    ui->OptionsTable->setCellWidget(row, EDIT_DLG_COL3, wrapDurEnableCb);

    KeyVal *kPtrdurEnableCb = &k;
    connect(durEnableCb, &QCheckBox::toggled, this, [kPtrdurEnableCb](bool checked){
        kPtrdurEnableCb->setRptEnable(checked);
    });


    ui->OptionsTable->setItem(row, EDIT_DLG_COL4, new QTableWidgetItem(QString::number(k.msgDur())));

    // Column 5: Repeat checkbox
    auto *rptEnableCb = new QCheckBox;
    rptEnableCb->setChecked(k.rptEnable());
    auto *wrapRptEnableCb = new QWidget;
    auto *layRptEnableCb = new QHBoxLayout(wrapRptEnableCb);
    layRptEnableCb->addWidget(rptEnableCb);
    layRptEnableCb->setAlignment(Qt::AlignCenter);
    layRptEnableCb->setContentsMargins(0,0,0,0);
    ui->OptionsTable->setCellWidget(row, EDIT_DLG_COL5, wrapRptEnableCb);

    KeyVal *kPtrRptEnableCB = &k;
    connect(rptEnableCb, &QCheckBox::toggled, this, [kPtrRptEnableCB](bool checked){
        kPtrRptEnableCB->setRptEnable(checked);
    });

    ui->OptionsTable->setItem(row, EDIT_DLG_COL6, new QTableWidgetItem(QString::number(k.rptDur())));

    // Column 7: Record/play button
    auto *recBtn = new QToolButton;
    recBtn->setText("🎙");
    recBtn->setToolTip(tr("Record/Play audio for this message"));
    recBtn->setFixedSize(24, 24);

    auto *wrapBtn = new QWidget;
    auto *layBtn = new QHBoxLayout(wrapBtn);
    layBtn->addWidget(recBtn);
    layBtn->setAlignment(Qt::AlignCenter);
    layBtn->setContentsMargins(0,0,0,0);
    ui->OptionsTable->setCellWidget(row, EDIT_DLG_COL7, wrapBtn);

    KeyVal *kPtrBtn = &k;
    connect(recBtn, &QToolButton::clicked, this, [this, kPtrBtn]() {
        if (txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
        {
            TxVmInternalButtonDialog dlg(this);
            dlg.exec();
        }
        else if (txKeyerType == keyerTypes[TxKeyerId::ExternalVoiceKeyer])
        {
            // handle external keyer
        }

        // Example: access KeyVal if needed
        QString top = kPtrBtn->ktop();
        QString val = kPtrBtn->kval();
    });
}


void DMKeysEditDlg::setupTableColumns()
{
    constexpr int totalColumns = 6;
    ui->OptionsTable->setColumnCount(totalColumns);

    ui->OptionsTable->setHorizontalHeaderLabels(
        { tr("Key Top"), tr("Value"), tr("Rig\nMem"),
         tr("Repeat"), tr("Repeat\nDur"), tr("Rec.") });

    auto *hh = ui->OptionsTable->horizontalHeader();

    hh->setStretchLastSection(false);
    hh->setSectionResizeMode(EDIT_DLG_COL0, QHeaderView::Interactive);
    hh->setSectionResizeMode(EDIT_DLG_COL1, QHeaderView::Interactive);
    hh->setSectionResizeMode(EDIT_DLG_COL2, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL3, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL4, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL5, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL6, QHeaderView::Fixed);
    hh->setSectionResizeMode(EDIT_DLG_COL7, QHeaderView::Fixed);


    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL0, 140);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL1, 220);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL2, 50);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL3, 50);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL4, 60);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL5, 30);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL6, 30);
    ui->OptionsTable->setColumnWidth(EDIT_DLG_COL7, 30);

    if (txKeyerType == keyerTypes[TxKeyerId::DigitalModes])
    {
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL2, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL3, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL4, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL5, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL6, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL7, true);
    }

    // Hide Unwanted colums for keyer Type

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL1, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL7, true);
    }

    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl] || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL2, true);
        ui->OptionsTable->setColumnHidden(EDIT_DLG_COL7, true);
    }

}

void DMKeysEditDlg::showDetails()
{
    ui->OptionsTable->clear();

    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend()) return;

    const auto contestIt = keyerIt->constFind(selectedContestName);
    if (contestIt == keyerIt->cend()) return;

    const auto &rigMap = contestIt.value();

    bool radioExists = false;
    if (checkRadioExists(selectedContestName, radioListSelectedName, radioExists))
    {
        if (!radioExists)
        {
            radioListSelectedName = KEYER_NO_RADIO;
            ui->deleteRadioButton->setEnabled(false);
        }

    }

    QString rigKey = radioListSelectedName;
    const ContestSection &sect = rigMap[rigKey];

    // Mutable copies of KeySets
    KeySet &run = const_cast<KeySet&>(sect.run);
    KeySet &sp  = const_cast<KeySet&>(sect.sp);

    if (run.size() != 12 || sp.size() != 12)
        return;

    // ---------- Table setup ----------
    ui->OptionsTable->setRowCount(24);
    setupTableColumns();

    for (int row = 0; row < 24; ++row)
    {
        KeyVal &k = (row < 12 ? run[row] : sp[row - 12]);
        setupTableRow(row, k);
    }

    // ---------- Restore splitter position ----------
    RegSettings settings;
    ui->settingsSplitter->restoreState(
        settings.getSettings().value("DMKeysEdit/SplitterState/" + selectedContestName).toByteArray());
}



void DMKeysEditDlg::getDetails()
{
    if (!ui->OptionsTable->rowCount())
        return;

    auto &contestMap = allKeyConfigs[txKeyerType][selectedContestName];

    // Locate rig key (use rigName, fallback if needed)
    QString rigKey = radioListSelectedName;


    ContestSection &section = contestMap[rigKey];

    if (section.run.isEmpty() && section.sp.isEmpty())
        return;

    int runCount = section.run.size();
    int totalRows = ui->OptionsTable->rowCount();

    for (int r = 0; r < totalRows; ++r)
    {
        KeyVal *entry = nullptr;
        if (r < runCount)
        {
            entry = &section.run[r];
        }
        else
        {
            int spIndex = r - runCount;
            if (spIndex < section.sp.size())
                entry = &section.sp[spIndex];
        }

        if (!entry)
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

        // Column 3: Duration Enable (checkbox)
        if (QWidget *wrap = ui->OptionsTable->cellWidget(r, EDIT_DLG_COL3))
        {
            if (QCheckBox *cb = wrap->findChild<QCheckBox*>())
                entry->setMsgDurEnable(cb->isChecked());
        }

        // Column 4: Repeat Duration
        if (QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL4))
        {
            bool ok;
            int dur = qtwi->text().toInt(&ok);
            if (ok)
                entry->setMsgDur(dur);
        }



        // Column 5: Repeat Enable (checkbox)
        if (QWidget *wrap = ui->OptionsTable->cellWidget(r, EDIT_DLG_COL5))
        {
            if (QCheckBox *cb = wrap->findChild<QCheckBox*>())
                entry->setRptEnable(cb->isChecked());
        }

        // Column 6: Repeat Duration
        if (QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL6))
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
    getDetails();  // Store current UI into model

}

bool DMKeysEditDlg::isCurrentSectionDirty() const
{
    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend()) return false;

    const auto contestIt = keyerIt->constFind(selectedContestName);
    if (contestIt == keyerIt->cend()) return false;

    QString rigKey = radioListSelectedName;

    const auto &section = contestIt->value(rigKey);

    for (const auto &k : section.run)
        if (k.isDirty()) return true;
    for (const auto &k : section.sp)
        if (k.isDirty()) return true;

    return false;
}

void DMKeysEditDlg::clearDirtyFlag()
{

    QString rigKey = radioListSelectedName;

    // Check if the path exists before accessing
    if (!allKeyConfigs.contains(txKeyerType))
        return;

    auto &contestMap = allKeyConfigs[txKeyerType];
    if (!contestMap.contains(selectedContestName))
        return;

    auto &rigMap = contestMap[selectedContestName];
    if (!rigMap.contains(rigKey))
        return;

    auto &section = rigMap[rigKey];
    for (auto &k : section.run)
        k.clearDirty();
    for (auto &k : section.sp)
        k.clearDirty();
}





//---------------------------------------------------------------------------

void DMKeysEditDlg::on_NewSectionButton_clicked()
{

    saveCurrentSection();

    QString newContestName = "new digi key section";

    if (enquireDialog(this, tr("Please give a new name for the %1").arg(selectedContestName), newContestName))
    {
        auto &contestMap = allKeyConfigs[txKeyerType];

        if (!contestMap.contains(newContestName))
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
            QString rigKey = radioListSelectedName;

            contestMap[newContestName][rigKey] = section;


            // Update current selection and UI
            ignoreSectionChange = true;
            selectedContestName = newContestName;
            showSections();
            showDetails();
            ignoreSectionChange = false;

        }
        else
        {
            mShowMessage(tr("%1 already exists").arg(newContestName), this);
        }
    }
}



void DMKeysEditDlg::on_CopyButton_clicked()
{
    //getDetails();  // Save current edits
    saveCurrentSection();

    //int offset = ui->SectionsList->currentRow();
    //if (offset <= 0)
    //    return;



    QString newContestName = "new digi key section";
    if (!enquireDialog(this, tr("Please give a name for the new %1").arg(selectedContestName), newContestName))
        return;

    auto &contestMap = allKeyConfigs[txKeyerType];

    if (contestMap.contains(newContestName))
    {
        mShowMessage(tr("%1 already exists").arg(newContestName), this);
        return;
    }

    QString rigKey = radioListSelectedName;

    // Check source exists
    if (!contestMap.contains(selectedContestName) || !contestMap[selectedContestName].contains(rigKey))
    {
        mShowMessage(tr("Cannot copy — source section missing."), this);
        return;
    }

    // Deep copy section
    contestMap[newContestName][rigKey] = contestMap[selectedContestName][rigKey];

    // Update UI
    selectedContestName = newContestName;
    ignoreSectionChange = true;
    showSections();
    showDetails();
    ignoreSectionChange = false;
}


void DMKeysEditDlg::on_DeleteButton_clicked()
{
    if (selectedContestName == "<None>"  || selectedContestName == "Default")
    {
        mShowMessage(tr("You cannot delete the empty %1!").arg(selectedContestName), this);
        return;
    }
    if (!mShowYesNoMessage(this, tr("Are you sure you want to delete the current %1?").arg(selectedContestName)))
        return;

    auto keyerIt = allKeyConfigs.find(txKeyerType);
    if (keyerIt == allKeyConfigs.end())
        return;

    auto &contestMap = keyerIt.value();

    // Simply remove the entire contest
    contestMap.remove(selectedContestName);

    if (contestMap.isEmpty())
        allKeyConfigs.remove(txKeyerType);  // Optional cleanup

    // Reset selection to <None> or first available
    if (!contestMap.isEmpty())
    {
        selectedContestName = contestMap.firstKey();
    }
    else
    {
        selectedContestName = QStringLiteral("<None>");
    }

    ignoreSectionChange = true;
    showSections();
    showDetails();
    ignoreSectionChange = false;
}



void DMKeysEditDlg::on_renameButton_clicked()
{

    saveCurrentSection();

    if (selectedContestName == "<None>"  || selectedContestName == "Default")
    {
        mShowMessage(tr("You cannot rename the empty %1!").arg(selectedContestName), this);
        return;
    }

    QString newContestName = selectedContestName;
    if (enquireDialog(this, tr("Please give a new name for the %1").arg(selectedContestName), newContestName))
    {
        auto &contestMap = allKeyConfigs[txKeyerType];

        if (!contestMap.contains(newContestName))
        {
            // Rename by moving value and removing old key
            contestMap[newContestName] = contestMap[selectedContestName];
            contestMap.remove(selectedContestName);

            selectedContestName = newContestName;

            ignoreSectionChange = true;
            showSections();
            showDetails();
            ignoreSectionChange = false;
        }
        else
        {
            mShowMessage(tr("%1 already exists").arg(newContestName), this);
        }
    }
}

void DMKeysEditDlg::on_addRadioButton_clicked()
{
    if (selectedContestName == "<None>"  || selectedContestName == "Default")
    {
        mShowMessage(tr("You cannot add radio to Default!"), this);
        return;
    }


    DmButtonEditAddRadioDialog* addRadioDialog = new DmButtonEditAddRadioDialog(listOfRadios);

    QString titleText;
    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        titleText = tr(" supporting Voice Keyer");
    }
    else if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
       titleText = tr(" supporting CW Keyer");
    }
    addRadioDialog->setWindowTitle(tr("Add Radio %1").arg(titleText));



    int result = addRadioDialog->exec();

    if (result == QDialog::Accepted)
    {
        bool radioExists = false;
        if (checkRadioExists(selectedContestName, addRadioDialog->getRadioName(), radioExists))
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
                    kv.setMsgDurEnable(false);
                    kv.setMsgDur(0);
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
                    kv.setMsgDurEnable(false);
                    kv.setMsgDur(0);
                    kv.setRptEnable(false);
                    kv.setRptDur(0);

                    section.sp.append(kv);
                }

                contestMap[selectedContestName][addRadioDialog->getRadioName()] = section;

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
    if (radioListSelectedName == KEYER_NO_RADIO)
    {
        mShowMessage(tr("You cannot delete noRadio!"), this);
        return;
    }


    if (!mShowYesNoMessage(this, tr("Are you sure you want to delete the radio %1?").arg(radioListSelectedName)))
    {

        return;


    }
    else
    {
        auto keyerIt = allKeyConfigs.find(txKeyerType);
        if (keyerIt != allKeyConfigs.end())
        {
            ContestMap &contestMap = keyerIt.value();
            auto contestIt = contestMap.find(selectedContestName);
            if (contestIt != contestMap.end())
            {
                RigMap &rigMap = contestIt.value();
                if (rigMap.contains(radioListSelectedName))
                {
                    rigMap.remove(radioListSelectedName);
                    qDebug() << "Deleted radio:" << radioListSelectedName;

                    if (!rigMap.isEmpty())
                        radioListSelectedName = rigMap.firstKey();
                    else
                        radioListSelectedName = QStringLiteral("<NoRadio>");
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

    saveCurrentSection();
    accept();
}

void DMKeysEditDlg::on_SectionsList_itemSelectionChanged()
{
    if (ignoreSectionChange)        // when making program changes
        return;

    saveCurrentSection();

    const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
    if (keyerIt == allKeyConfigs.cend() || ui->SectionsList->currentRow() < 0)
        return;

    QStringList sections = keyerIt->keys();

    int offset = ui->SectionsList->currentRow();
    if (offset >= 0 && offset < sections.size())
    {
        selectedContestName = sections[offset];
    }

    if (RadioList)
    {
        RadioList->clear();

        const auto contestIt = keyerIt->constFind(selectedContestName);  // name = selected section/contest
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

    radioListSelectedName = KEYER_NO_RADIO;       // set default radio
    showSection();
}



void DMKeysEditDlg::onRadioListItemSelectionChanged()
{
    if (ignoreSectionChange)        // when making program changes
        return;

    saveCurrentSection();

    if (RadioList)
    {

        const auto keyerIt = allKeyConfigs.constFind(txKeyerType);
        if (keyerIt == allKeyConfigs.cend())
            return;

        const auto contestIt = keyerIt->constFind(selectedContestName);  // name = selected section/contest
        if (contestIt == keyerIt->cend())
            return;

        // Extract rig names for this contest
        QStringList radioNames = contestIt->keys();

        int offset = RadioList->currentRow();
        if (offset >= 0 && offset < radioNames.size())
        {
            radioListSelectedName = radioNames[offset];
            showSection();

            if (radioListSelectedName == KEYER_NO_RADIO)
            {
                ui->deleteRadioButton->setEnabled(false);
            }
            else
            {
                ui->deleteRadioButton->setEnabled(true);
            }
        }
    }
}

void DMKeysEditDlg::closeEvent(QCloseEvent *event)
{
    doCloseEvent();  // Save geometry + splitter
    QDialog::closeEvent(event);
}


void DMKeysEditDlg::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("DMKeysEdit/geometry/" + txKeyerType, saveGeometry());
    settings.getSettings().setValue("DMKeysEdit/SplitterState/" + txKeyerType, ui->settingsSplitter->saveState());
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
    if (!contestMap.contains(selectedContestName))
        return;

    QString rigKey = radioListSelectedName;
    if (!contestMap[selectedContestName].contains(rigKey))
        return;

    ContestSection &section = contestMap[selectedContestName][rigKey];

    int runCount = section.run.size();
    int totalCount = runCount + section.sp.size();
    if (selRow >= totalCount)
        return;


    if (selRow < runCount && selRow - 1 < runCount)
    {
        // Both in run section
        section.run[selRow].swapWith(section.run[selRow - 1]);
    }
    else if (selRow >= runCount && selRow - 1 >= runCount)
    {
        // Both in sp section
        int spRow = selRow - runCount;
        int spRowPrev = selRow - 1 - runCount;

        section.sp[spRow].swapWith(section.sp[spRowPrev]);
    }
    else if (selRow == runCount)
    {

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
    if (!contestMap.contains(selectedContestName))
        return;

    QString rigKey = radioListSelectedName;
    if (!contestMap[selectedContestName].contains(rigKey))
        return;

    ContestSection &section = contestMap[selectedContestName][rigKey];

    int runCount = section.run.size();
    int totalCount = runCount + section.sp.size();
    if (selRow >= totalCount - 1)
    {
        return;
    }

    if (selRow < runCount - 1)
    {
        // Both in run section
        section.run[selRow].swapWith(section.run[selRow + 1]);
    }
    else if (selRow >= runCount && selRow + 1 >= runCount)
    {
        // Both in sp section
        int spRow = selRow - runCount;
        int spRowNext = selRow + 1 - runCount;
        section.sp[spRow].swapWith(section.sp[spRowNext]);
    }
    else if (selRow == runCount - 1)
    {
        // Moving last run row down to first sp row - swap across boundary
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

bool DMKeysEditDlg::checkRadioExists(QString contestName, QString radioName, bool &radioExists)
{

    bool ok = false;

    if (allKeyConfigs.contains(txKeyerType))
    {
        const ContestMap &contestMap = allKeyConfigs.value(txKeyerType);

        ok = true;

        if (contestMap.contains(contestName))
        {
            const RigMap &rigMap = contestMap.value(contestName);

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





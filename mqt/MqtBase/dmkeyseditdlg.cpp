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
#include "ui_dmkeyseditdlg.h"
#include "txkeyerCommonConstants.h"

using namespace TxKeyerCommon;


DMKeysEditDlg::DMKeysEditDlg(QWidget *parent, QString fKeyFileName, QString name, Keys &keys, QString txKeyerType) :
    QDialog(parent),
    ui(new Ui::DMKeysEditDlg),
    keys(keys),
    name(name),
    txKeyerType(txKeyerType)
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
    QStringList sections;

    sections = keys.keys();

    std::sort(sections.begin(), sections.end(),
              [](const QString &first, const QString &second) { return sectionLessThan(first, second); });

    int offset = 0;
    for ( int i = 0; i < sections.size(); i++ )
    {
        if ( sections[ i ] == name )
            offset = i;
        ui->SectionsList->addItem( sections[ i ] );
    }
    ui->SectionsList->setCurrentRow(offset);
    showSection( );
}
//---------------------------------------------------------------------------
void DMKeysEditDlg::showSection()
{
    // Select this section to display on tree
    QStringList sections = keys.keys();
    int offset = ui->SectionsList->currentRow();

    if ( offset >= 0 && offset < sections.size() )
    {
        std::sort(sections.begin(), sections.end(),
                  [](const QString &first, const QString &second) { return sectionLessThan(first, second); });

        ui->OptionsTable->setVisible(true);
        showDetails();
    }

    else
    {
        ui->OptionsTable->setVisible(false);
    }

    ui->NewSectionButton->setEnabled(true);
    ui->DeleteButton->setEnabled( offset > 0 );
    ui->CopyButton->setEnabled(offset > 0 );
    ui->renameButton->setEnabled(offset > 0 );

}

void DMKeysEditDlg::showDetails()
{
    ui->OptionsTable->clear();
    int offset = ui->SectionsList->currentRow();


    if (offset >= 0)
    {
        const int totalColumns = 6;
        ui->OptionsTable->setColumnCount(totalColumns);
        ui->OptionsTable->setRowCount(keys[name].size());

        QStringList vHeaders;

        for (int i = 0; i < keys[name].size(); ++i)
        {
            vHeaders << QString("%1 F%2")
            .arg(i < 12 ? tr("Run") : tr("S&P"))
                .arg(i < 12 ? i + 1   : (i - 12) + 1);


            ui->OptionsTable->setItem(i, EDIT_DLG_COL0, new QTableWidgetItem(keys[name][i].ktop));
            ui->OptionsTable->setItem(i, EDIT_DLG_COL1, new QTableWidgetItem(keys[name][i].kval));

            // (col-2) - map radio voice mem
            ui->OptionsTable->setItem(i, EDIT_DLG_COL2, new QTableWidgetItem(QString::number(keys[name][i].rigVoiceMemNum)));

            // Checkbox (col-3) - repeat enable
            auto *cb  = new QCheckBox;
            cb->setChecked(keys[name][i].rptEnable);
            auto *wcb = new QWidget;
            auto *lcb = new QHBoxLayout(wcb);
            lcb->addWidget(cb);
            lcb->setAlignment(Qt::AlignCenter);
            lcb->setContentsMargins(0,0,0,0);
            ui->OptionsTable->setCellWidget(i, EDIT_DLG_COL3, wcb);

            //  (col-4) - repeat pause duration
            ui->OptionsTable->setItem(i, EDIT_DLG_COL4, new QTableWidgetItem(QString::number(keys[name][i].rptDur)));

            // record button (col-5)
            QToolButton *recBtn = new QToolButton();
            recBtn->setText("🎙");
            recBtn->setToolTip(tr("Record/Play audio for this message"));
            recBtn->setFixedSize(24, 24);
            recBtn->setProperty("row", i);
            QWidget *wrap = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout(wrap);
            layout->addWidget(recBtn);
            layout->setAlignment(Qt::AlignCenter);
            layout->setContentsMargins(0, 0, 0, 0);
            wrap->setLayout(layout);
            ui->OptionsTable->setCellWidget(i, EDIT_DLG_COL5, wrap);
            connect(recBtn, &QToolButton::clicked, this, [this, recBtn](){
                int row = recBtn->property("row").toInt();

            });



        }

        ui->OptionsTable->setVerticalHeaderLabels(vHeaders);
        ui->OptionsTable->setHorizontalHeaderLabels(
            { tr("Key Top"), tr("Value"), tr("Rig\nMem"), tr("Repeat"), tr("Repeat\nDur"), tr("Rec.") });


        auto *hh = ui->OptionsTable->horizontalHeader();
        hh->setStretchLastSection(false);                      // don’t stretch col-5

        hh->setSectionResizeMode(EDIT_DLG_COL0, QHeaderView::Interactive);     // Key Top
        hh->setSectionResizeMode(EDIT_DLG_COL1, QHeaderView::Interactive);     // Value
        hh->setSectionResizeMode(EDIT_DLG_COL2, QHeaderView::Fixed);        // map rig voice mem num
        hh->setSectionResizeMode(EDIT_DLG_COL3, QHeaderView::Fixed);       // repeat enable
        hh->setSectionResizeMode(EDIT_DLG_COL4, QHeaderView::Fixed);       // repeat pause duration
        hh->setSectionResizeMode(EDIT_DLG_COL5, QHeaderView::Fixed);       // rec toolbutton

        ui->OptionsTable->setColumnWidth(EDIT_DLG_COL0, 140);
        ui->OptionsTable->setColumnWidth(EDIT_DLG_COL1, 220);
        ui->OptionsTable->setColumnWidth(EDIT_DLG_COL2, 50);
        ui->OptionsTable->setColumnWidth(EDIT_DLG_COL3, 50);
        ui->OptionsTable->setColumnWidth(EDIT_DLG_COL4, 60);
        ui->OptionsTable->setColumnWidth(EDIT_DLG_COL5, 30);


    }
/*
    // hide unwanted colums
    if (txKeyerType == TxKeyerCommon::keyerTypes[TxKeyerCommon::TxKeyerId::DigitalModes])
    {
        ui->OptionsTable->setColumnHidden(2, true);
        ui->OptionsTable->setColumnHidden(3, true);
    }
    else if (txKeyerType == TxKeyerCommon::keyerTypes[TxKeyerCommon::TxKeyerId::RigControl])
    {
        ui->OptionsTable->setColumnHidden(1, true);
    }
*/




    /* restore splitter state, etc. */
    RegSettings settings;
    ui->settingsSplitter->restoreState(
        settings.getSettings().value("DMKeysEdit/SplitterState/" + name).toByteArray());
}


void DMKeysEditDlg::getDetails()
{
    if (ui->OptionsTable->rowCount())
    {
        for ( int r = 0; r < keys[name].size(); r++ )
        {
            QTableWidgetItem *qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL0);

            if (qtwi)
            {
                QString val = qtwi->text();
                keys[name][r].ktop = val ;
            }
            qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL1);
            if (qtwi)
            {
                QString val = qtwi->text();
                keys[name][r].kval = val ;
            }

            qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL2);
            if (qtwi)
            {
                bool ok;
                int rigVoiceMem =  qtwi->text().toInt(&ok);

                if(ok)
                {
                    keys[name][r].rigVoiceMemNum = rigVoiceMem;
                }
            }

            //int rigVoiceMem = 0;    // default
            //if (auto *le1 = qobject_cast<QLineEdit*>(ui->OptionsTable->cellWidget(r, EDIT_DLG_COL2)))
            //{
            //    bool ok;
            //    rigVoiceMem = le1->text().toInt(&ok);
            //    if (ok)
            //    {

            //    }

            //}


            bool val = false;
            if (QWidget *wrap = ui->OptionsTable->cellWidget(r, EDIT_DLG_COL3))
            {
                if (QCheckBox *cb = wrap->findChild<QCheckBox*>())
                {
                    val = cb->isChecked();
                    keys[name][r].rptEnable = val ;
                }
            }

            qtwi = ui->OptionsTable->item(r, EDIT_DLG_COL4);
            if (qtwi)
            {
                bool ok;
                int dur =  qtwi->text().toInt(&ok);

                if(ok)
                {
                    keys[name][r].rptDur = dur;
                }
            }


            //int dur = 0;    // default
            //if (auto *le2 = qobject_cast<QLineEdit*>(ui->OptionsTable->cellWidget(r, EDIT_DLG_COL4)))
            //{
            //    bool ok;
            //    dur = le2->text().toInt(&ok);
            //    if (ok)
            //    {
           //        keys[name][r].rptDur = dur;
           //     }

           // }



        }
    }
}

//---------------------------------------------------------------------------

void DMKeysEditDlg::on_NewSectionButton_clicked()
{
    getDetails();  // save what is set already
    QString Value = "new digi key section" ;

    if ( enquireDialog( this, tr("Please give a new name for the %1").arg(name), Value ) )
    {
        if (!keys.contains(Value))
        {
            keys[Value]; // create the section

            // add blank values for all keys
            for (auto mode:{"Run", "SandP"})
            {
                Q_UNUSED(mode)
                for (int i = 1; i <= 12; i++)
                {
                    QString fk = QString("F%1").arg(i);
                    QString keytop;
                    QString val;
                    int rigVoiceMemNum = 0;
                    bool rptEnable = false;
                    int rptDur = 0;


                    KeyVal kv;
                    kv.fk = fk;
                    kv.ktop = keytop;
                    kv.kval = val;
                    kv.rigVoiceMemNum = rigVoiceMemNum;
                    kv.rptEnable = rptEnable;
                    kv.rptDur = rptDur;


                    keys[Value].append(kv);
                }
            }
            showSections();
            showDetails();
        }
        else
        {
            mShowMessage( tr("%1 already exists").arg(Value), this );
        }
    }
}

void DMKeysEditDlg::on_CopyButton_clicked()
{
    getDetails();  // save what is set already

    int offset = ui->SectionsList->currentRow();
    if ( offset > 0)
    {
        QString Value = "new digi key section" ;
        if ( enquireDialog( this, tr("Please give a name for the new %1").arg(name), Value ) )
        {
            getDetails();  // save old section

            if (!keys.contains(Value))
            {
                keys[Value]; // create the section
                for ( int r = 0; r < keys[name].size(); r++ )
                {
                    keys[Value].push_back(keys[name][r]);
                }

                showSections();
                showDetails();
            }
            else
            {
                mShowMessage( tr("%1 already exists").arg(name), this );
            }
        }
    }
}

void DMKeysEditDlg::on_DeleteButton_clicked()
{
    //   getDetails();  // we are about to delete it - don't save it first!

    // delete the section - don't allow "<None>" to be deleted!
    // First, put up an "Are you sure?" message

    int offset = ui->SectionsList->currentRow();
    if ( offset == 0 )
        mShowMessage( tr("You cannot delete the empty %1!").arg(name), this );
    else
        if ( mShowYesNoMessage( this, tr("Are you sure you want to delete the current %1?").arg(name ) ))
        {
            keys.remove(name);
            showSections();
            showDetails();
        }
}

void DMKeysEditDlg::on_renameButton_clicked()
{
    getDetails();  // save what is set already

    int offset = ui->SectionsList->currentRow();
    if ( offset == 0 )
        mShowMessage( tr("You cannot rename the empty %1!").arg( name ), this );
    else if ( offset > 0 )
    {
        QString Value = name;
        if ( enquireDialog( this, tr("Please give a new name for the %1").arg(name), Value ) )
        {
            getDetails();  // save old section

            if (!keys.contains(Value))
            {
                keys[Value] = keys[name];
                keys.remove(name);
                name = Value;
                showSections();
                showDetails();
            }
            else
            {
                mShowMessage( tr("%1 already exists").arg(name), this );
            }
        }
    }
}

void DMKeysEditDlg::on_CancelButton_clicked()
{
    reject();
}

void DMKeysEditDlg::on_OKButton_clicked()
{
    getDetails();
    accept();
}

void DMKeysEditDlg::on_SectionsList_itemSelectionChanged()
{
    getDetails();  // save what is set already
    QStringList sections = keys.keys();
    int offset = ui->SectionsList->currentRow();
    name = sections[offset];
    showSection();
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
    // move Key content up one
    QModelIndexList mil = ui->OptionsTable->selectionModel()->selectedRows();
    if (mil.count() == 1 && mil[0].row() != 0)
    {
        int selRow = mil[0].row();

        std::swap(keys[name][selRow].ktop, keys[name][selRow - 1].ktop);
        std::swap(keys[name][selRow].kval, keys[name][selRow - 1].kval);

        std::swap(keys[name][selRow].rigVoiceMemNum, keys[name][selRow - 1].rigVoiceMemNum);
        std::swap(keys[name][selRow].rptEnable, keys[name][selRow - 1].rptEnable);
        std::swap(keys[name][selRow].rptDur, keys[name][selRow - 1].rptDur);

        showDetails();

        ui->OptionsTable->selectRow(selRow - 1);
    }
}


void DMKeysEditDlg::on_downButton_clicked()
{
    // move key content down one
    QModelIndexList mil = ui->OptionsTable->selectionModel()->selectedRows();
    if (mil.count() == 1 && mil[0].row() != ui->OptionsTable->rowCount() - 1)
    {
        int selRow = mil[0].row();

        std::swap(keys[name][selRow].ktop, keys[name][selRow + 1].ktop);
        std::swap(keys[name][selRow].kval, keys[name][selRow + 1].kval);

        std::swap(keys[name][selRow].rigVoiceMemNum, keys[name][selRow + 1].rigVoiceMemNum);
        std::swap(keys[name][selRow].rptEnable, keys[name][selRow + 1].rptEnable);
        std::swap(keys[name][selRow].rptDur, keys[name][selRow + 1].rptDur);

        showDetails();
        ui->OptionsTable->selectRow(selRow + 1);
    }
}


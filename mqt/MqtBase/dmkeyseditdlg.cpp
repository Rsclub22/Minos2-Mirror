#include <QSettings>
#include <QString>
#include <QPushButton>

#include "regsettings.h"
#include "enqdlg.h"
#include "MShowMessageDlg.h"
#include "MMessageDialog.h"
#include "dmkeyseditdlg.h"
#include "ui_dmkeyseditdlg.h"


DMKeysEditDlg::DMKeysEditDlg(QWidget *parent, QString fKeyFileName, QString name, Keys &keys) :
    QDialog(parent),
    ui(new Ui::DMKeysEditDlg),
    keys(keys),
    name(name)
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

    ui->NewSectionButton->setText(tr("New Digi FKey section"));
    ui->CopyButton->setText(tr("Copy Digi FKey section"));
    ui->DeleteButton->setText(tr("Delete Digi FKey section"));
    ui->renameButton->setText(tr("Rename Digi FKey section"));

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
    if ( offset >= 0 )
    {
        QStringList fkeys;

        ui->OptionsTable->setColumnCount(2);
        ui->OptionsTable->setRowCount(keys[name].size());

        for ( int i= 0; i < keys[name].size(); i++ )
        {
            fkeys.append(QString("%1 F%2").arg((i<12)?tr("Run"):tr("S&P")).arg((i < 12)?i + 1:(i - 12) + 1));

            QString top = keys[name][i].ktop ;

            QTableWidgetItem *it = new QTableWidgetItem(top);
            ui->OptionsTable->setItem(i, 0, it);

            QString val =  keys[name][i].kval ;

            it = new QTableWidgetItem(val);
            ui->OptionsTable->setItem(i, 1, it);
        }
        ui->OptionsTable->setVerticalHeaderLabels(fkeys);
        ui->OptionsTable->setHorizontalHeaderLabels({tr("Key Top"), tr("Value")});
    }

    RegSettings settings;
    QByteArray state = settings.getSettings().value("DMKeysEdit/SplitterState/" + name).toByteArray();
    ui->settingsSplitter->restoreState(state);
}
void DMKeysEditDlg::getDetails()
{
    if (ui->OptionsTable->rowCount())
    {
        for ( int r = 0; r < keys[name].size(); r++ )
        {
            QTableWidgetItem *qtwi = ui->OptionsTable->item(r, 0);

            if (qtwi)
            {
                QString val = qtwi->text();
                keys[name][r].ktop = val ;
            }
            qtwi = ui->OptionsTable->item(r, 1);
            if (qtwi)
            {
                QString val = qtwi->text();
                keys[name][r].kval = val ;
            }
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

                    KeyVal kv;
                    kv.fk = fk;
                    kv.ktop = keytop;
                    kv.kval = val;

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

        showDetails();
        ui->OptionsTable->selectRow(selRow + 1);
    }
}


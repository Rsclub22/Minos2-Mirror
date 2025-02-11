#include <QSettings>
#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFileSystemWatcher>

#include "regsettings.h"
#include "enqdlg.h"

#include "dmkeyseditdlg.h"
#include "ui_dmkeyseditdlg.h"

DMKeysEditDlg::DMKeysEditDlg(QWidget *parent, QString name, Keys &keys) :
    QDialog(parent),
    ui(new Ui::DMKeysEditDlg),
    keys(keys),
    name(name)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    initialSection = name;

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("DMKeysEdit/geometry/" + name).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state = settings.getSettings().value("DMKeysEdit/SplitterState/" + name).toByteArray();
    ui->settingsSplitter->restoreState(state);

    baseTitle = windowTitle();

    ui->NewSectionButton->setText(tr("New %1").arg(name));
    ui->CopyButton->setText(tr("Copy %1").arg(name));
    ui->DeleteButton->setText(tr("Delete %1").arg(name));
    ui->renameButton->setText(tr("Rename %1").arg(name));

    ui->SectionsList->setMinimumWidth(10);
    ui->OptionsTable->setMinimumWidth(10);
}
void DMKeysEditDlg::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
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

   sections = keys["Digi"].keys();

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
   lastOffset = offset;
   showSection( );
}
//---------------------------------------------------------------------------
void DMKeysEditDlg::showSection()
{

   // Select this section to display on tree
   QStringList sections = keys["Digi"].keys();
   int offset = ui->SectionsList->currentRow();
   lastOffset = offset;

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
   setWindowTitle(baseTitle + " - " + name) ;

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
       ui->OptionsTable->setRowCount(keys["Digi"][name].size());

       for ( int i= 0; i < keys["Digi"][name].size(); i++ )
       {
           fkeys.append(QString("%1 F%2").arg((i<12)?"Run":"S&P").arg((i < 12)?i + 1:(i - 12) + 1));
           QString label = keys["Digi"][name][i].first ;

           QTableWidgetItem *it = new QTableWidgetItem(label);
           ui->OptionsTable->setItem(i, 0, it);

           QString val =  keys["Digi"][name][i].second ;

          it = new QTableWidgetItem(val);
          ui->OptionsTable->setItem(i, 1, it);
       }
       ui->OptionsTable->setVerticalHeaderLabels(fkeys);
   }

}
void DMKeysEditDlg::getDetails()
{
    /*

      from

https://stackoverflow.com/questions/1332110/selecting-qcombobox-in-qtablewidget

for (each row in table ... ) {
   QComboBox* combo = new QComboBox();
   table->setCellWidget(row,col,combo);
   combo->setCurrentIndex(node.type());
   connect(combo, &QComboBox::currentIndexChanged,this, &JsonEditDlg::changed));
   ....
}

Also
When the combobox is created you can simply add two custom properties to it:

combo->setProperty("row", (int) nRow);
combo->setProperty("col", (int) nCol);
In the handler function you can get a pointer back to the sender of the signal (your combobox).

Now by asking for the properties you can have your row/col back:

int nRow = sender()->property("row").toInt();
int nCol = sender()->property("col").toInt();
      */

//   int offset = ui->SectionsList->currentRow();
    int offset = lastOffset;
   if (ui->OptionsTable->rowCount())
   {
      for ( int r = 0; r < keys["Digi"][name].size(); r++ )
      {
          QTableWidgetItem *qtwi = ui->OptionsTable->item(r, 0);
          if (qtwi)
          {
            QString val = qtwi->text();
            keys["Digi"][name][offset].first = val ;
          }
          qtwi = ui->OptionsTable->item(r, 1);
          if (qtwi)
          {
              QString val = qtwi->text();
              keys["Digi"][name][offset].second = val ;
          }
      }
   }
}

//---------------------------------------------------------------------------

void DMKeysEditDlg::on_NewSectionButton_clicked()
{
#ifdef RUBBISH

   getDetails();  // save what is set already
   QString Value = "new " + bundle->getBundle();

   if ( enquireDialog( this, tr("Please give a new name for the %1").arg(name, Value ) )
   {
      if (bundle->newSection( Value ))
      {
          bundle->openSection( Value );
          showSections(Value);
          showDetails();
      }
      else
      {
          MinosParameters::getMinosParameters() ->mshowMessage( tr("%1 already exists").arg(Value), this );
      }
   }
#endif
}

void DMKeysEditDlg::on_CopyButton_clicked()
{
#ifdef RUBBISH

    getDetails();  // save what is set already

    int offset = ui->SectionsList->currentRow();
    if ( offset > 0 && !currSectionOnly)
    {
       QString Value = bundle->getSection();
       if ( enquireDialog( this, tr("Please give a name for the new %1").arg(name, Value ) )
       {
          getDetails();  // save old section

          if ( bundle->dupSection( Value ) )
          {
             bundle->openSection( Value );
             showSections(Value);
             showDetails();
          }
          else
          {
             MinosParameters::getMinosParameters() ->mshowMessage( tr("%1 already exists").arg(name, this );
          }
       }
    }
#endif
}

void DMKeysEditDlg::on_DeleteButton_clicked()
{
#ifdef RUBBISH

    //   getDetails();  // we are about to delete it - don't save it first!

    // delete the section - don't allow "<None>" to be deleted!
    // First, put up an "Are you sure?" message

    int offset = ui->SectionsList->currentRow();
    if ( offset == 0 )
       MinosParameters::getMinosParameters() ->mshowMessage( tr("You cannot delete the empty %1!").arg(name, this );
    else
       if ( MinosParameters::getMinosParameters() ->yesNoMessage( this, tr("Are you sure you want to delete the current %1?").arg(name ) )
       {
          bundle->clearProfileSection( true );
          showSections(bundle->getSection());
          showDetails();
       }
#endif
}

void DMKeysEditDlg::on_renameButton_clicked()
{
#ifdef RUBBISH

    getDetails();  // save what is set already

    int offset = ui->SectionsList->currentRow();
    if ( offset == 0 )
       MinosParameters::getMinosParameters() ->mshowMessage( tr("You cannot rename the empty %1!").arg( name, this );
    else if ( offset > 0 && !currSectionOnly)
    {
       QString Value = bundle->getSection();
       if ( enquireDialog( this, tr("Please give a new name for the %1").arg(name, Value ) )
       {
          getDetails();  // save old section

          if ( bundle->renameSection( Value ) )
          {
             bundle->openSection( Value );
             showSections(Value);
             showDetails();
          }
          else
          {
             MinosParameters::getMinosParameters() ->mshowMessage( tr("%1 already exists").arg(name, this );
          }
       }
    }
#endif
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


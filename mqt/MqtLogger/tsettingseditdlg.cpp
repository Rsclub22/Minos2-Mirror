#include "logger_pch.h"

#include "tsettingseditdlg.h"
#include "ui_tsettingseditdlg.h"
#include "enqdlg.h"

TSettingsEditDlg::TSettingsEditDlg(QWidget *parent, SettingsBundle *bundle) :
    QDialog(parent),
    ui(new Ui::TSettingsEditDlg),
    bundle(bundle),
    currSectionOnly(false)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    initialSection = bundle->getSection();

    QSettings settings;
    QByteArray geometry = settings.value("EntrySettings/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    baseTitle = windowTitle();
}
void TSettingsEditDlg::ShowCurrentSectionOnly()
{
   currSectionOnly = true;
}
int TSettingsEditDlg::exec()
{
    showSections(bundle->getSection());
    showDetails();

    return QDialog::exec();
}

TSettingsEditDlg::~TSettingsEditDlg()
{
    delete ui;
}
bool sectionLessThan(SettingsBundle *b, const QString &s1, const QString &s2)
{
    if (s1 == b->noneBundle)
        return true;
    if (s2 == b->noneBundle)
        return false;
    return s1.toLower() < s2.toLower();
}
void TSettingsEditDlg::showSections(QString currSection)
{
   ui->SectionsList->clear();
   QStringList sections;

   if (currSectionOnly)
   {
      sections.append(initialSection);
   }
   else
   {
      sections = bundle->getSections( );
   }


   qSort(sections.begin(), sections.end(),// sectionLessThan);
             [this](const QString &first, const QString &second) { return sectionLessThan(bundle, first, second); });
   setWindowTitle(baseTitle + " - " + bundle->getBundle() + " for " + bundle->getSection()) ;

   int offset = 0;
   for ( int i = 0; i < sections.size(); i++ )
   {
      if ( sections[ i ] == currSection )
         offset = i;
      ui->SectionsList->addItem( sections[ i ] );
   }
   ui->SectionsList->setCurrentRow(offset);
   showSection( );
}
//---------------------------------------------------------------------------
void TSettingsEditDlg::showSection()
{
   // Select this section to display on tree
   QStringList sections = bundle->getSections( );
   int offset = ui->SectionsList->currentRow();
   if (currSectionOnly)
   {
      bundle->openSection( initialSection );
      ui->OptionsTable->setVisible(true);
      showDetails();
   }
   else if ( offset >= 0 && offset < sections.size() )
   {
      qSort(sections.begin(), sections.end(),// sectionLessThan);
           [this](const QString &first, const QString &second) { return sectionLessThan(bundle, first, second); });
      QString sect = sections[ offset ];
      bundle->openSection( sect );
      ui->OptionsTable->setVisible(true);
      showDetails();
   }

   else
   {
      ui->OptionsTable->setVisible(false);
      bundle->openSection( bundle->noneBundle );
   }

   ui->NewSectionButton->setEnabled(!currSectionOnly);
   ui->DeleteButton->setEnabled(currSectionOnly?false:( offset > 0 ));  // ************* >= ?
   ui->CopyButton->setEnabled(currSectionOnly?false:( offset > 0 )); // ************* >= ?
   ui->renameButton->setEnabled(currSectionOnly?false:( offset > 0 )); // ************* >= ?
}
void TSettingsEditDlg::showDetails()
{
   ui->OptionsTable->clear();
   int offset = ui->SectionsList->currentRow();
   if ( offset >= 0 || currSectionOnly)
   {
       bundle->startGroup();
       // sections are all i = aaaaaaa
       QVector<int> entries = bundle->getBundleEntries();

       QStringList hints = bundle->getBundleHints( );

       QStringList labels;
       QStringList values;

       ui->OptionsTable->setColumnCount(1);
       ui->OptionsTable->setRowCount(entries.size());

       for ( int i= 0; i < entries.size(); i++ )
       {
           QString label = bundle->displayNameOf( entries[ i ] );
           labels.append(label);

           QString val;
           if (bundle->populateDefaultSection() || offset > 0 || currSectionOnly)               // ********** >= ?
            bundle->getStringProfile( entries[ i ], val );

           QTableWidgetItem *it = new QTableWidgetItem(val);
           it->setToolTip(hints[i]);


          bool RO = bundle->isReadOnly(entries[ i ]);
          if (RO)
          {
             Qt::ItemFlags fl = it->flags();
             fl &= Qt::ItemIsEditable;
             it->setFlags(fl);
          }
          ui->OptionsTable->setItem(i, 0, it);
       }
       ui->OptionsTable->setVerticalHeaderLabels(labels);
       bundle->endGroup();
   }
}
void TSettingsEditDlg::getDetails()
{
    /*

      from

https://stackoverflow.com/questions/1332110/selecting-qcombobox-in-qtablewidget

for (each row in table ... ) {
   QComboBox* combo = new QComboBox();
   table->setCellWidget(row,col,combo);
   combo->setCurrentIndex(node.type());
   connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(changed(int)));
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

   int offset = ui->SectionsList->currentRow();
   if (( offset > 0 || currSectionOnly) && ui->OptionsTable->rowCount())  // ************* >= ?
   {
      QVector<int> entries = bundle->getBundleEntries();
      for ( int r = 0; r < entries.size(); r++ )
      {
          QTableWidgetItem *qtwi = ui->OptionsTable->item(r, 0);
          if (qtwi)
          {
            QString val = qtwi->text();
            bundle->setStringProfile( entries[ r ], val );
          }
      }
   }
}

//---------------------------------------------------------------------------

void TSettingsEditDlg::on_NewSectionButton_clicked()
{
    getDetails();  // save what is set already
    QString Value = "new section";

    bundle->newSection( Value );
    bundle->openSection( Value );
    showSections(Value);
    showDetails();

    on_renameButton_clicked();
}

void TSettingsEditDlg::on_CopyButton_clicked()
{
    getDetails();  // save what is set already

    int offset = ui->SectionsList->currentRow();
    if ( offset > 0 && !currSectionOnly) // ************* >= ?
    {
       QString Value = bundle->getSection();
       if ( enquireDialog( this, "Please give a name for the new section", Value ) )
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
             MinosParameters::getMinosParameters() ->mshowMessage( "Section already exists", this );
          }
       }
    }
}

void TSettingsEditDlg::on_DeleteButton_clicked()
{
    //   getDetails();  // we are about to delete it - don't save it first!

    // delete the section - don't allow "<None>" to be deleted!
    // First, put up an "Are you sure?" message

    int offset = ui->SectionsList->currentRow();
    if ( offset == 0 )
       MinosParameters::getMinosParameters() ->mshowMessage( "You cannot delete the empty section!", this );
    else
       if ( MinosParameters::getMinosParameters() ->yesNoMessage( this, "Are you sure you want to delete the current section?" ) )
       {
          bundle->clearProfileSection( true );
          showSections(bundle->getSection());
          showDetails();
       }
}

void TSettingsEditDlg::on_renameButton_clicked()
{
    getDetails();  // save what is set already

    int offset = ui->SectionsList->currentRow();
    if ( offset == 0 )
       MinosParameters::getMinosParameters() ->mshowMessage( "You cannot rename the empty section!", this );
    else if ( offset > 0 && !currSectionOnly) // ************* >= ?
    {
       QString Value = bundle->getSection();
       if ( enquireDialog( this, "Please give a new name for the section", Value ) )
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
             MinosParameters::getMinosParameters() ->mshowMessage( "Section already exists", this );
          }
       }
    }
}

void TSettingsEditDlg::on_CancelButton_clicked()
{
    bundle->openSection( initialSection );
    reject();
}

void TSettingsEditDlg::on_OKButton_clicked()
{
    getDetails();
    bundle->flushProfile();
    accept();
}

void TSettingsEditDlg::on_SectionsList_itemSelectionChanged()
{
    getDetails();  // save what is set already
    showSection();
}
void TSettingsEditDlg::doCloseEvent()
{
    QSettings settings;
    settings.setValue("EntrySettings/geometry", saveGeometry());
}
void TSettingsEditDlg::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TSettingsEditDlg::accept()
{
    doCloseEvent();
    QDialog::accept();
}

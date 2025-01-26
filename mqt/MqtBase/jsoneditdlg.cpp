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
#include "profiles.h"
#include "enqdlg.h"
#include "MinosParameters.h"

#include "jsoneditdlg.h"
#include "ui_jsoneditdlg.h"

JsonEditDlg::JsonEditDlg(QWidget *parent, QString name, QJsonDocument *bundle) :
    QDialog(parent),
    ui(new Ui::JsonEditDlg),
    bundle(bundle),
    currSectionOnly(false),
    name(name)

{
#ifdef RUBBISH
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    initialSection = bundle->getSection();

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("JsonEdit/geometry/" + name).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state = settings.getSettings().value("JsonEdit/SplitterState/" + name).toByteArray();
    ui->settingsSplitter->restoreState(state);

    baseTitle = windowTitle();

    ui->NewSectionButton->setText(tr("New %1").arg(name));
    ui->CopyButton->setText(tr("Copy %1").arg(name));
    ui->DeleteButton->setText(tr("Delete %1").arg(name));
    ui->renameButton->setText(tr("Rename %1").arg(name));

    ui->SectionsList->setMinimumWidth(10);
    ui->OptionsTable->setMinimumWidth(10);
#endif
}
void JsonEditDlg::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->settingsSplitter->saveState();
    settings.getSettings().setValue("JsonEdit/SplitterState/" + name, state);
}
void JsonEditDlg::ShowCurrentSectionOnly()
{
   currSectionOnly = true;
}
int JsonEditDlg::exec()
{
#ifdef RUBBISH

    showSections(bundle->getSection());
#endif
    showDetails();

    return QDialog::exec();
}

JsonEditDlg::~JsonEditDlg()
{
    delete ui;
}
#ifdef RUBBISH

bool sectionLessThan(SettingsBundle *b, const QString &s1, const QString &s2)
{
    if (s1 == b->noneBundle)
        return true;
    if (s2 == b->noneBundle)
        return false;
    return s1.toLower() < s2.toLower();
}
#endif
void JsonEditDlg::showSections(QString currSection)
{
#ifdef RUBBISH

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


   std::sort(sections.begin(), sections.end(),
             [this](const QString &first, const QString &second) { return sectionLessThan(bundle, first, second); });

   int offset = 0;
   for ( int i = 0; i < sections.size(); i++ )
   {
      if ( sections[ i ] == currSection )
         offset = i;
      ui->SectionsList->addItem( sections[ i ] );
   }
   ui->SectionsList->setCurrentRow(offset);
   showSection( );
#endif
}
//---------------------------------------------------------------------------
void JsonEditDlg::showSection()
{
#ifdef RUBBISH

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
      std::sort(sections.begin(), sections.end(),
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
   setWindowTitle(baseTitle + " - " + bundle->getBundle() + (currSectionOnly?QString():tr(" for \"") + bundle->getSection() + "\"")) ;

   if (currSectionOnly)
   {
       ui->NewSectionButton->setVisible(false);
       ui->DeleteButton->setVisible(false);
       ui->CopyButton->setVisible(false);
       ui->renameButton->setVisible(false);
       ui->SectionsList->setVisible(false);
   }
   else
   {
       ui->NewSectionButton->setEnabled(true);
       ui->DeleteButton->setEnabled( offset > 0 );
       ui->CopyButton->setEnabled(offset > 0 );
       ui->renameButton->setEnabled(offset > 0 );
   }
#endif
}
void JsonEditDlg::showDetails()
{
#ifdef RUBBISH

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
           if (bundle->populateDefaultSection() || offset > 0 || currSectionOnly)
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
#endif
}
void JsonEditDlg::getDetails()
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
#ifdef RUBBISH

   int offset = ui->SectionsList->currentRow();
   if (( offset > 0 || currSectionOnly) && ui->OptionsTable->rowCount())
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
#endif
}

//---------------------------------------------------------------------------

void JsonEditDlg::on_NewSectionButton_clicked()
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

void JsonEditDlg::on_CopyButton_clicked()
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

void JsonEditDlg::on_DeleteButton_clicked()
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

void JsonEditDlg::on_renameButton_clicked()
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

void JsonEditDlg::on_CancelButton_clicked()
{
#ifdef RUBBISH

    bundle->openSection( initialSection );
    reject();
#endif
}

void JsonEditDlg::on_OKButton_clicked()
{
#ifdef RUBBISH

    getDetails();
    bundle->flushProfile();
    accept();
#endif
}

void JsonEditDlg::on_SectionsList_itemSelectionChanged()
{
    getDetails();  // save what is set already
    showSection();
}
void JsonEditDlg::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("JsonEdit/geometry/" + name, saveGeometry());
}
void JsonEditDlg::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void JsonEditDlg::accept()
{
    doCloseEvent();
    QDialog::accept();
}


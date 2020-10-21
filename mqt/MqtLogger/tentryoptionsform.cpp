#include "base_pch.h"
#include "LoggerContest.h"
#include "ContestDetailsTransferObject.h"
#include "tentryoptionsform.h"
#include "ui_tentryoptionsform.h"


TEntryOptionsForm::TEntryOptionsForm(QWidget* Owner, QSharedPointer<ContestDetailsTransferObject> cnt, LoggerContestLog *inputContest, bool saveMinos ):
    QDialog(Owner),
    ui(new Ui::TEntryOptionsForm),
    ct( cnt ), inputContest(inputContest),
    minosSave( saveMinos )
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (!ct && inputContest)
    {
        ct = QSharedPointer<ContestDetailsTransferObject>(new ContestDetailsTransferObject);
        ct->getFromContest(inputContest);
    }
    if ( !ct )
        return ;

    QSettings settings;
    QByteArray geometry = settings.value("EntryOptions/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    QStringList vlabels =
    {
        tr("Date Range (Calculated)"),
        tr("Contest Name"),
        tr("Band"),
        tr("Band Points Multiplier"),
        tr("Entrant name (or group)"),
        tr("Station QTH 1"),
        tr("Station QTH 2"),
        tr("Section"),
        tr("Callsign as sent"),
        tr("Locator as sent"),
        tr("Exchange/code/QTH as sent"),
        tr("Transmitter"),
        tr("Transmit Power"),
        tr("Receiver"),
        tr("Antenna"),
        tr("Height above ground"),
        tr("Height above sea level"),
        tr("(From QSOs) Operators Line 1"),
        tr("(From QSOs) Operators Line 2"),
        tr("(Entry)Operators Line 1"),
        tr("(Entry)Operators Line 2"),
        tr("Conditions/Comments"),
        tr("Conditions/Comments"),
        tr("Conditions/Comments"),
        tr("Conditions/Comments"),
        tr("Name for Correspondence"),
        tr("Callsign for Correspondence"),
        tr("Address 1 for Correspondence"),
        tr("Address 2 for Correspondence"),
        tr("City for Correspondence"),
        tr("Country for Correspondence"),
        tr("Postcode for Correspondence"),
        tr("Phone number for queries"),
        tr("email address for queries")
    };

    ui->OptionsScrollBox->horizontalHeader()->hide();
    ui->OptionsScrollBox->verticalHeader()->hide();

    ui->OptionsScrollBox->setColumnCount(2);
    ui->OptionsScrollBox->setRowCount(vlabels.count());

    int r = 0;

    ui->OptionsScrollBox->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->OptionsScrollBox->setVerticalHeaderLabels(vlabels);

    QTableWidgetItem *dateRangeItem = new QTableWidgetItem(ct->dateRange);
    dateRangeItem->setFlags(dateRangeItem->flags() ^ Qt::ItemIsEditable);
    dateRangeItem->setBackground(QBrush(Qt::lightGray));

    ui->OptionsScrollBox->setItem(r++, 0, dateRangeItem);
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->name.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->contestBands.getValue()));
    int bpm = ct->bandPointsMultiplier.getValue();
    if (bpm == 0)
        bpm = 1;
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(QString::number(bpm)));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entrant.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->sqth1.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->sqth2.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entSect.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->mycall.getFullCall()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->myloc.getLoc()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->location.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entTx.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->power.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entRx.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entAnt.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entAGL.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entASL.getValue()));

    getContestOperators();
    QTableWidgetItem *opsQSOLine1 = new QTableWidgetItem(ct->opsQSO1);
    opsQSOLine1->setFlags(opsQSOLine1->flags() ^ Qt::ItemIsEditable);
    opsQSOLine1->setBackground(QBrush(Qt::lightGray));
    ui->OptionsScrollBox->setItem(r++, 0, opsQSOLine1);
    QTableWidgetItem *opsQSOLine2 = new QTableWidgetItem(ct->opsQSO2);
    opsQSOLine2->setFlags(opsQSOLine2->flags() ^ Qt::ItemIsEditable);
    opsQSOLine2->setBackground(QBrush(Qt::lightGray));
    ui->OptionsScrollBox->setItem(r++, 0, opsQSOLine2);

    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->ops1.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->ops2.getValue()));

    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCondx1.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCondx2.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCondx3.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCondx4.getValue()));

    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entName.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCall.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entAddr1.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entAddr2.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCity.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entCountry.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entPostCode.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entPhone.getValue()));
    ui->OptionsScrollBox->setItem(r++, 0, new QTableWidgetItem(ct->entEMail.getValue()));

    ui->OptionsScrollBox->verticalHeader()->show();

    if ( minosSave )
    {
        // we have loaded from an alien format, and are about to export
        // OR we are looking at contest details, and no export will be honoured
        ui->enrb3->setChecked( true );
        ui->EntryGroupBox->setVisible( false );
        ui->NACSerials->setVisible( false );
    }
    else
    {
        ui->enrb0->setChecked( true );
    }
}

TEntryOptionsForm::~TEntryOptionsForm()
{
    delete ui;
}
void TEntryOptionsForm::doCloseEvent()
{
    QSettings settings;
    settings.setValue("EntryOptions/geometry", saveGeometry());
}
void TEntryOptionsForm::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TEntryOptionsForm::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void TEntryOptionsForm::on_CloseButton_clicked()
{
    int r = 0;
    r++;  // date range not editable

    ct->name.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->contestBands.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    int bpm = ui->OptionsScrollBox->item(r++, 0)->text().toInt();
    if (bpm == 0)
        bpm = 1;
    ct->bandPointsMultiplier.setValue(bpm);
    ct->entrant.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->sqth1.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->sqth2.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entSect.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->mycall.setFullCall( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->myloc.setLoc( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->location.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entTx.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->power.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entRx.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entAnt.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entAGL.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entASL.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );

    ct->opsQSO1 = ui->OptionsScrollBox->item(r++, 0)->text() ;
    ct->opsQSO2 = ui->OptionsScrollBox->item(r++, 0)->text() ;

    ct->ops1.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->ops2.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );

    ct->entCondx1.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entCondx2.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entCondx3.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entCondx4.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entName.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entCall.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entAddr1.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entAddr2.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entCity.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entCountry.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entPostCode.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entPhone.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );
    ct->entEMail.setValue( ui->OptionsScrollBox->item(r++, 0)->text() );

    accept();
}

void TEntryOptionsForm::on_CancelButton_clicked()
{
    reject();
}
//---------------------------------------------------------------------------
void TEntryOptionsForm::getContestOperators()
{
    OperatorList operators;

    for ( OperatorIterator op = ct->oplist.begin(); op != ct->oplist.end(); op++ )
    {
        operators.insert(*op, *op);
    }

    // now actual ops are a sorted list

    QString ops1;
    QString ops2;
    int ls = operators.size();
    int i = 0;
    foreach(QString op, operators)
    {
        if (i <= ls/2)
        {
            ops1 += op + " ";
        }
        else
        {
            ops2 += op + " ";
        }
        i++;
    }
    ct->opsQSO1 = ops1.trimmed();
    ct->opsQSO2 = ops2.trimmed();
}

QString TEntryOptionsForm::doFileSave( )
{
    QString expName;
    QVector<ExportType> exptypes;
    if (ui->enrb0->isChecked())
    {
        exptypes.push_back(EREG1TEST);
    }
    if (ui->enrb1->isChecked())
    {
        exptypes.push_back(EADIF);
    }
    if (ui->enrb2->isChecked())
    {
        exptypes.push_back(EG0GJV);
    }
    if (ui->enrb3->isChecked())
    {
        exptypes.push_back(EMINOS);
    }
    if (ui->enrb4->isChecked())
    {
        exptypes.push_back(EKML);
    }
    if (ui->enrb5->isChecked())
    {
        exptypes.push_back(EPRINTFILE);
    }
    if (ui->enrb6->isChecked())
    {
        exptypes.push_back(ECABRILLO);
    }

    foreach(ExportType expformat, exptypes)
    {
        QString InitialDir = ExtractFilePath( ct->cfileName );

        QString fname = ExtractFileName( ct->cfileName );
        QString ext = ExtractFileExt( ct->cfileName );
        fname = fname.left( fname.size() - ext.size() );

        //enum ExportType {EREG1TEST, ECABRILLO, EADIF, EG0GJV, EMINOS, EKML, EPRINTFILE };
        QString defext;
        QString filter;
        QString title;

        switch ( expformat )
        {
        case EG0GJV:
            defext = "gjv";
            filter = tr("GJV ContestLog files (*.gjv);;All Files (*.*);;") ;
            title = tr("Save contest in GJV format as...");
            break;
        case EMINOS:
            defext = "minos";
            filter = tr("Minos ContestLog files (*.minos *.Minos);;All Files (*.*);;") ;
            title = tr("Save contest in .minos format as...");
            break;
        case EADIF:
            defext = "adi";
            filter = tr("ADIF files (*.adi);;All Files (*.*);;") ;
            title = tr("Save contest in ADIF format as...");
            break;
        case EKML:
            defext = "kml";
            filter = tr("KML(GoogleEarth) files (*.kml);;All Files (*.*);;") ;
            title = tr("Save contest in KML(GoogleEarth) format as...");
            break;
        case EREG1TEST:
            defext = "edi";
            filter =tr( "Region 1 EDI files (*.edi);;All Files (*.*);;") ;
            title = tr("Save contest in Region 1 EDI file format as...");
            break;
        case EPRINTFILE:
        {
            defext = "txt";
            filter = tr("Text output (*.txt);;All Files (*.*);;" );
            title = tr("Save contest in printable text format as...");
            break;
        }
        case ECABRILLO:
        {
            defext = "cbr";
            filter =tr( "Cabrillo files (*.cbr);;All Files (*.*);;") ;
            title = tr("Save contest in Cabrillo file format as...");
            break;
        }
        }
        bool Ok = false;

        while (!Ok)
        {
            QString fileName = QFileDialog::getSaveFileName( this,
                                                             title,
                                                             InitialDir + "/" + fname + "." + defext,
                                                             filter);
    //                                                         ,
    //                                                         0,
    //                                                         QFileDialog::DontConfirmOverwrite
    //                                                         );
            if ( !fileName.isEmpty() )
            {

                expName = fileName;

                // open the export file
                if ( FileAccessible(expName) && !FileWriteable(expName) )
                {
                        MinosParameters::getMinosParameters() ->mshowMessage( tr("File is Read Only"), this );
                        continue;
                }
                if ( MinosParameters::getMinosParameters() ->isContestOpen( expName ) )
                {
                    // then try again...
                    continue;
                }

                QIODevice::OpenMode om = QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered;
                QSharedPointer<QFile> contestFile(new QFile(expName));

                if (!contestFile->open(om))
                {
                   QString lerr = contestFile->errorString();
                   QString emess = tr("Failed to open Contest export file%1 : %2 ").arg(expName).arg(lerr);
                   MinosParameters::getMinosParameters() ->mshowMessage( emess, this );
                   continue;
                }


                ct->setToContest(inputContest);
                int ret = inputContest->export_contest( contestFile, expformat, ui->NACSerials->isChecked() );
                contestFile->close();

                if ( ret == -1 )
                {
                    QFile::remove( expName );		// failure response, so delete file again
                    expName = "";
                }
                break;
            }
            else
            {
                break;
            }
        }
    }
    return expName;
}

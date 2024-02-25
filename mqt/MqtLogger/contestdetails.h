#ifndef CONTESTDETAILS_H
#define CONTESTDETAILS_H

#include <QDialog>
#include "Calendar.h"
#include "focuswatcher.h"
#include "ContestDetailsTransferObject.h"
#include "cutils.h"

namespace Ui {
class ContestDetails;
}

class LoggerContestLog;
class IndividualContest;

class ContestDetails : public QDialog
{
    Q_OBJECT

    static const char * BSHelpText;
public:
    explicit ContestDetails(QWidget *parent = nullptr);
    ~ContestDetails() override;

private:
    Ui::ContestDetails *ui;
    QString trAllHf;
    UpperCaseValidator ucValidator;

    QSharedPointer< ContestDetailsTransferObject > contestTransferObject;
   LoggerContestLog * inputcontest;
   QString sectionList;
   bool saveContestOK;
   bool suppressProtectedOnClick;
   bool noMultRipple;

   FocusWatcher *ContestNameEditFW;
   FocusWatcher *BandComboBoxFW;
   FocusWatcher *CallsignEditFW;
   FocusWatcher *LocatorEditFW;
   FocusWatcher *ExchangeEditFW;
   FocusWatcher *ExchangeComboBoxFW;
   FocusWatcher *PowerEditFW;
   FocusWatcher *MainOpComboBoxFW;

   FocusWatcher *allowLoc4FW;
   FocusWatcher *allowLoc8FW;

   QMap<QString, QCheckBox*> allBandChkBoxMap;

   QWidget * getNextFocus();
   void setDetails( const IndividualContest &ic );
   void setDetails( );
   void setModes();
   void refreshOps();
   void enableControls();
   QWidget * getDetails( );

   void doCloseEvent();

   void setExchangeComboBox();
   
   QString getSelectedRadio();
   void setSelectedRadio(QString );
   QString getSelectedAntenna();
   void setSelectedAntenna(QString s);

   void setBandBoxes(QString bandStr, QString bandsList);
   void loadClubNames(QString groupName);
   void doCalendarButton(QString dtitle, CalType calt);
   public Q_SLOTS:
   virtual void accept() override;
   virtual void reject() override;

public:   		// User declarations
   void setDetails( LoggerContestLog * ct );
   void bundleChanged();

   virtual int exec() override;
private slots:
   void focusChange(QObject *, bool, QFocusEvent *event);
   void on_OKButton_clicked();
   void on_EntDetailButton_clicked();
   void on_CancelButton_clicked();
   void on_HFCalendarButton_clicked();
   void on_VHFCalendarButton_clicked();
   void on_uwaveCalendarButton_clicked();
   void on_CallsignEdit_editingFinished();
   void on_DXCCMult_clicked();
   void on_LocatorMult_clicked();
   void on_ProtectedOption_clicked();
   void on_BonusComboBox_currentIndexChanged(int index);
   void on_MGMCheckBox_stateChanged(int arg1);
   void on_RotatorList();
   void on_SetRadioList();
   void on_ageProtectedcb_stateChanged(int arg1);
   void on_BandComboBox_activated(int arg1);
   void on_ExchangeComboBox_activated(int arg1);
   void on_UKSMGCalendarButton_clicked();
   void on_BARTGCalendarButton_clicked();
};

#endif // CONTESTDETAILS_H

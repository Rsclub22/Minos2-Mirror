#ifndef CONTESTDETAILS_H
#define CONTESTDETAILS_H

#include <QDialog>
#include "tbundleframe.h"
#include "focuswatcher.h"
#include "ContestDetailsTransferObject.h"

namespace Ui {
class ContestDetails;
}

class LoggerContestLog;
class IndividualContest;

class ContestDetails : public QDialog
{
    Q_OBJECT

public:
    explicit ContestDetails(QWidget *parent = nullptr);
    ~ContestDetails() override;

private:
    Ui::ContestDetails *ui;

private:   	// User declarations
   QSharedPointer< ContestDetailsTransferObject > contest;
   LoggerContestLog * inputcontest;
   QString sectionList;
   bool saveContestOK;
   bool suppressProtectedOnClick;
   bool noMultRipple;

   FocusWatcher *ContestNameEditFW;
   FocusWatcher *BandComboBoxFW;
   FocusWatcher *CallsignEditFW;
   FocusWatcher *LocatorEditFW;
   FocusWatcher *PowerEditFW;
   FocusWatcher *MainOpComboBoxFW;

   QWidget * getNextFocus();
   void setDetails( const IndividualContest &ic );
   void setDetails( );
   void refreshOps();
   void enableControls();
   QWidget * getDetails( );

   void doCloseEvent();

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
   void on_BSHelpButton_clicked();
   void on_VHFCalendarButton_clicked();
   void on_CallsignEdit_editingFinished();
   void on_DXCCMult_clicked();
   void on_NonGCtryMult_clicked();
   void on_LocatorMult_clicked();
   void on_GLocMult_clicked();
   void on_M7LocatorMults_clicked();
   void on_ProtectedOption_clicked();
   void on_BonusComboBox_currentIndexChanged(int index);
   void on_MGMCheckBox_stateChanged(int arg1);
   void on_RotatorList();
   void on_SetRadioList();
};

#endif // CONTESTDETAILS_H

#ifndef QSOLOGFRAME_H
#define QSOLOGFRAME_H

#include "contacts.h"
#include "ScreenContact.h"
#include "focuswatcher.h"
#include "validators.h"
#include "cutils.h"
#include "rigmemcommondata.h"

class ListContact;
class ContactList;

namespace Ui {
class QSOLogFrame;
}

class QSOLogFrame : public QFrame
{
    Q_OBJECT
    Ui::QSOLogFrame *ui;

public:
    explicit QSOLogFrame(QWidget *parent);
    ~QSOLogFrame() override;

    void setAsEdit(bool s, QString b);

    bool savePartial(  );
    bool restorePartial( );
    void killPartial( );
    void startNextEntry( );
    bool doKeyPressEvent( QKeyEvent* event );
    virtual void selectEntryForEdit(QSharedPointer<BaseContact> lct );
    virtual void sortUnfilledCatchupTime( );

    virtual void getScreenEntry();
    ScreenContact screenContact;  // contact being edited on screen
    void calcLoc( );

    QSharedPointer<BaseContact> selectedContact;   // contact from log list selected
    bool catchup = false;
    bool unfilled = false;
    QString sentExchange;

    bool setActiveControl( int *Key, Qt::KeyboardModifiers mods );
    void clearCurrentField();
    void lgTraceerr( int err );

    virtual void selectField( QWidget *v );

    virtual void selectCallField();
    virtual void selectSnRxField();
    virtual void selectExchField();

    virtual void initialise();
    virtual void setContest(BaseContestLog * contest);
    virtual void setTimeStyles();
    virtual void refreshOps();
    virtual void refreshOps(ScreenContact &screenContact);
    virtual void updateQSOTime(bool fromTimer = false);
    void setDtgSection();
    virtual void updateQSODisplay();

    void doGJVCancelButton_clicked();
    void doGJVOKButton_clicked();

    void transferDetails(CheckableContact *lct, const BaseContestLog *matct );
    void transferDetails(const ListContact *lct, const ContactList *matct );
    void transferDetails(QString cs, const QString loc, QString exchange, const bool fromBandmapOrMemory );

    void logTabChanged();

    void modeSentFromRig(QString mode);
    void setFreq(Frequency freq);
    void setRadioName(QString);
    QString getRadioName();
    void setRadioState(QString s);
    void setRotatorBearing(const QString &s);

    QString getBearing();

    bool valid( validTypes command );

    ScreenContact *getPartialContact() const;
    void setPartialContact(ScreenContact *value);

    void setClusterTXSpotEnableState(bool txEnableState);

    void setRunOnFlag(bool runModeOn);
    void setRunOffFreqFlag(bool offRunFreq);

    void transferFromWSJTX(QString call);
    void setQrzButtonVisible(bool state);
    void setqrzDisplayFrameLoaded(bool loaded);

    void setPlaceholders(QStringList nearMatches);

    void transferFromQrz(QString callsign, QString locator, QString name);
    void selectFirstInvalid();
    void rxDMWord(QString rxWord, int markFreq);
    void DMKey(int key);
    bool getSandP();
private:
    ScreenContact *partialContact; // contact being edited on screen
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    UpperCaseValidator ucValidator;

    QString baseName;
    QString oldloc;
    bool locValid;

    bool oldTimeOK;
    QString timerSS;

    int markOffset = 0;

    void EditControlEnter( QObject *Sender, QFocusEvent *event );
    void EditControlExit( QObject *Sender );

    virtual void logScreenEntry( );
    virtual void logCurrentContact( );
    virtual void doGJVEditChange(QObject * );

    void setScoreText( int dist, bool partial, bool xband );
    bool dlgForced();
    bool validateControls( validTypes command );
    void contactValid( );

    void do_mouseDoubleClickEvent(QObject *w);
    QSharedPointer<BaseContact> getLastContact();
    QSharedPointer<BaseContact> getPriorContact();
    QSharedPointer<BaseContact> getNextContact();

    QVector <ValidatedControl *> vcs;

    BaseContestLog * contest = nullptr;
    bool overstrike = false;
    QWidget *current = nullptr;

    bool QRZControlsVisible = false;
    bool bandmapControlsVisible = false;
    bool clusterControlsVisible = false;

    void doAutofill( );
    void fillRst(QLineEdit *rIl, QString &rep, const QString &mode );
    void fillExchange(QLineEdit *rIl, const QString &exch);

    virtual void showScreenEntry( );
    virtual void getScreenContactTime();
    virtual void showScreenContactTime( );
    virtual void getScreenRigData();
    virtual void getscreenRotatorData();
    bool checkAndLogEntry( );

    bool edit = false;

    bool expert = false;
    bool altFKeys = false;

    QString mySentLabelString;
    QString theirSentLabelString;

    FocusWatcher *CallsignFW;
    QString CallsignLabelString;

    FocusWatcher *RSTTXFW;
    QString RSTTXLabelString;

    FocusWatcher *SerTXFW;
    QString SerTXLabelString;

    FocusWatcher *RSTRXFW;
    QString RSTRXLabelString;

    FocusWatcher *SerRXFW;
    QString SerRXLabelString;

    FocusWatcher *LocFW;
    QString LocLabelString;

    FocusWatcher *QTHFW;
    QString QTHLabelString;

    FocusWatcher *CommentsFW;
    QString CommentsLabelString;

    FocusWatcher *MainOpFW;
    QString Op1String;

    FocusWatcher *SecondOpFW;
    QString Op2String;

    FocusWatcher *freqFW;

    ErrorList errs;

    ValidatedControl *csIl;
    ValidatedControl *rsIl, *ssIl, *rrIl, *srIl;
    ValidatedControl *locIl;
    ValidatedControl *qthIl;
    ValidatedControl *cmntIl;

    ValidatedControl *freqIl;

    QWidget *getNextInvalid(QWidget *&firstInvalid);

    bool isRotatorLoaded();

    bool isRadioLoaded();

    void setBandMapControlsVisible(bool visible);
    bool logDataFromBandmapOrMemory;

    QMap<QString, int> addToBandmapTuneTolerance;
    QMap<QString, bool> addToBandmapTuneEnabled;

    bool qrzDisplayFrameLoaded;
    bool isQrzDisplayFrameLoaded();

    bool radioConnected;
    bool radioError;

    void setClusterSendSpotControlsVisible(bool visible);
    bool sendSpotToClusterOn;

    void MainOpComboBox_Exit();
    void SecondOpComboBox_Exit();

    bool runButtonOnFlag;
    bool radioOffRunFreq;

    Frequency callsignEnterTextFreq;

    QString mode;
    QString oldMode;
    bool qsoLogModeFlag = false;
    Frequency curFreq;
    QString curRadioName;
    QString curRotatorBearing;

    void setModes();
    void setOtherMode();
    void setMode(QString m);

    QString ssQsoFrameBlue = " #qsoFrame { border: 2px solid blue; }";
    QString ssQsoFrameRed = " #qsoFrame { background-color: lightcoral ;border: 2px solid red; }";
    QString ssRed = "color:red";

    QString ssDtgWhite = "QDateTimeEdit { background-color: white ; border-width: 1px ; border-color: black ; color: black ; }";
    QString ssDtgWhiteNoFrame = "QDateTimeEdit { background-color: white ; border : none ; color: black ; }";
    QString ssDtgRedNoFrame = "QDateTimeEdit { background-color: white ; border: none ; color: red ; }";
    QString ssDtgRed = "QDateTimeEdit { background-color: white ; border-width: 1px ; border-color: red ; color: red ; }";

    QString ssLineEditGreyBackground = "QLineEdit { background-color: silver ; border-style: outset ; border-width: 1px ; border-color: black ; color : black ;}";
    QString ssLineEditOK = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: black ; color : black ; }";
    QString ssLineEditNewMultPartial = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: red ; color : green ; }";
    QString ssLineEditNewMultFull = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: black ; color : green ; }";

    QString ssLineEditFrRedBkRed = "QLineEdit { background-color: red ; border-style: outset ; border-width: 1px ; border-color: red ; color : white }";
    QString ssLineEditFrRedBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: red ; color : black}";
    QString ssLineEditFrLightRedBkBk = "QLineEdit { background-color: pink ; border-style: outset ; border-width: 1px ; border-color: red ; color : black }";


    QMap<QWidget *, QString> widgetStyles;

    void checkBandMapAndClusterLoaded();

    void getLogDetails(memoryData::memData&, int &callRes);

    Callsign lastLoggedCallsign;        // saved to send to cluster
    QString lastLoggedLocator;
    Frequency lastLoggedFreq;


    void onBandMapAfterLogContact();
    bool chkRadioFreqOnRunFreq();
    void initLogRunButton();
    void runButtonOn();
    void runButtonOff();

    QString getRunMemoryFreq(int memoryNumber);

    void setBandmapControlsState();
    void setClusterSendSpotControlsState();
    void setBandMapControlsDisabled(bool disabled);
    void setClusterSendSpotControlsDisabled(bool disabled);
    void checkQsoFrameColour();
    
    bool frameHasFocus();
    void checkQrzDisplayFrameLoaded();
    void checkQRZClusterBandmapShowing();
    void doShowOperators(bool so);
    
    bool readTuneAddBandMapSetting(QString mode);
    QString getFKeyLabel(int n);
    void setEditStyleSheet(QLineEdit *qle, QString ss);
    void doBandmapSaveFreq(bool PbClicked);

signals:
    void QSOFrameCancelled();
    void sendBandMap( Frequency freq, QString call, QString utc, QString loc, QString qth );
    void sendModeControl(QString);
    void bandmapMarkFreq(Frequency, QString);
    void bandmapSaveFreq(QString, Frequency, QString, QString, QString, QString);
    void sendFreqControl(Frequency);
    void freqChanged(Frequency);
    void sendSpotToClusterServer(Frequency, QString, QString);
    void qrzCallsignRequest(QString);

private slots:
    void focusChange(QObject *, bool, QFocusEvent *event);
    void on_CatchupButton_clicked();
    void on_FirstUnfilledButton_clicked();
    void on_GJVOKButton_clicked();
    void on_GJVForceButton_clicked();
    void on_GJVCancelButton_clicked();
    void on_MatchXferButton_clicked();
    void onQTHEdit_textChanged(const QString &arg1);
    void onCallsignEdit_textChanged(const QString &arg1);
    void onLocEdit_textChanged(const QString &arg1);
    void on_ModeButton_clicked();
    void on_InsertBeforeButton_clicked();
    void on_InsertAfterButton_clicked();
    void on_PriorButton_clicked();
    void on_NextButton_clicked();

    void on_TimeDisplayTimer();
    void on_AfterTabFocusIn(QLineEdit *tle);
    void on_ValidateError (int mess_no );
    void on_ShowOperators();
    void on_tabSandP();
    void on_FontChanged();
    void on_QSOMargins();

    void on_ModeComboBoxGJV_activated(int index);
    void onRSTTXEdit_textChanged(const QString &arg1);
    void onRSTRXEdit_textChanged(const QString &arg1);
    void onSerRXEdit_textChanged(const QString &);
    void onSerTXEdit_textChanged(const QString &);
    void on_frequencyEdit_textChanged(const QString &arg1);

    void on_BandmapMarkFreqPbClicked();
    void on_bandmapSaveFreqPbClicked();

    void on_SpotPbClicked();
    void on_SpotLastLoggedPbClicked();

    void on_FreqChanged(Frequency f);

    void onQrzButtonClicked();
    void on_callRb_clicked();

    void on_SandPrb_clicked();
    void onContestBandChanged(BaseContestLog *c);
public slots:
    void setXferEnabled(bool s, BaseContestLog *c, QString basename);

};

#endif // QSOLOGFRAME_H

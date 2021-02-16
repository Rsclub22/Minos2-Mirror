#ifndef TLOGCONTAINER_H
#define TLOGCONTAINER_H

#include "base_pch.h"
#include "helpbrowser.h"
#include "n1mmbroadcast.h"
#include "serialtvswitch.h"

namespace Ui {
class TLogContainer;
}

class ContestDetails;
class BaseContestLog;
class BaseContact;
class TSingleLogFrame;
class TContactListDetails;
class ContactList;
class MatchContact;
class TSendDM;
class ContestPageControl;

class SetMemoryAction: public QAction
{
public:
    SetMemoryAction(QString t, QObject *p);
    BaseContestLog *ct = nullptr;
    QString call;
    QString loc;
};
class TLogContainer : public QMainWindow
{
    Q_OBJECT

public:
    explicit TLogContainer(QWidget *parent = nullptr);
    ~TLogContainer() override;

    TSendDM *sendDM;
    N1MMBroadcast n1mmBroadcast;

    void selectContest(BaseContestLog *pc, QSharedPointer<BaseContact> pct );
    bool show(int argc, char *argv[]);
    TSingleLogFrame *getCurrentLogFrame();

    QLabel *sblabel0;
    QLabel *sblabel1;
    QLabel *sblabel2;

    QMenu TabPopup;

    SerialTVSwitch *serialTVSw;

    TSingleLogFrame *findContest( const QString &pubname );
    TSingleLogFrame *findContest(BaseContestLog *ct );
    QVector<TSingleLogFrame *> getLogFrames();
    int getLogFrameCount();

    QVector<ContestPageControl *> contestPageControls;
    SetMemoryAction *setMemoryAction;

    static QString getDefaultDirectory( bool IsList );

    bool isShowOperators();

    bool isLoggerClosing()
    {
        return loggerClosing;
    }
    void setCaption( QString );

    QString getCurrSession();
    void setCurrSessionName(QString sessionName);

    QStringList getSessions();
    void updateSessionActions();
    void closeSession();
    void selectSession(QString sessName);

    void applyScreenLayouts();
    void selectLayout(QString layout);

    QAction *newAction(int n, QMenu *m, const char *atype);
    void doListOpenActionExecute(QWidget *p);
    void setMenuLog(int current);
    void selectContest(BaseContestLog *pc);
    void selectTab(int t);

private:
    Ui::TLogContainer *ui;

    QTimer TimerUpdateQSOTimer;

    QMap<QAction *, const char *> actionList;
    QMap<QMenu *, const char *> menuList;

    QMenu *screenLayoutMenu;
    enum { MaxRecentFiles = 5 };
    QVector<QAction *> recentFileActs;
    QMenu *recentFilesMenu;
    QVector<QAction *> sessionActs;
    QMenu *sessionsMenu;
    QMenu *keyerRecordMenu;
    QMenu *keyerPlaybackMenu;

    QSharedPointer<HelpBrowser>  helpBrowser;

    bool loggerClosing = false;

    void enableActions();

    QString getCurrentFile();
    void setCurrentFile(const QString &fileName);
    void removeCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    BaseContestLog * addSlot(ContestDetails *ced, const QString &fname, bool newfile, int slotno );
    void closeSlot(int t, bool addToMRU );
    TSingleLogFrame *findLogFrame(int t);

    QAction *lastSessionSelected = nullptr;
    QAction *lastLayoutSelected = nullptr;
    QAction *lastLanguageSelected = nullptr;

    QAction *newAction(const char *text, QMenu *m, const char *atype );
    QMenu *newMenu(QMenu *m, const char *text);
    SetMemoryAction *newMemoryAction(const char *text, QMenu *m, const char *atype );
    QAction *newCheckableAction(const char *text, QMenu *m, const char *atype );
    QAction *newCheckableAction(const QString text, QMenu *m, const char *atype );
    void setupMenus();

    QAction *HelpAction;
    QAction *HelpAboutAction;

    QAction *FileOpenAction;
    QAction *FileImportAction;
    QAction *ListOpenAccept;
    QAction *ContestDetailsAction;
    QAction *FileCloseAction;
    QAction *CloseAllAction;
    QAction *CloseAllButAction;
    QAction *OptionsAction;

    QAction *sessionManagerAction;

    QAction *ExitAction;
    QAction *ExitClearAction;
    QAction *MakeEntryAction;
    QAction *AppendAdifAction;
    QAction *FileNewAction;
    QAction *ListOpenAction;
    QAction *ManageListsAction;
    QAction *ShiftTabLeftAction;
    QAction *ShiftTabRightAction;

    QAction *startConfigAction;
    QAction *LocCalcAction;
    QAction *AnalyseMinosLogAction;
    QAction *CorrectDateTimeAction;
    QAction *ScreenConfigAction;
    QAction *AdvancedOptionsAction;

    QAction *GoToSerialAction;
    QAction *NextUnfilledAction;

    QAction *KeyerToneAction;
    QAction *KeyerTwoToneAction;
    QAction *KeyerStopAction;

    QAction *KeyerRecordAction;
    QAction *KeyerPlaybackAction;

    QVector< QSharedPointer<QAction> > menuLogsActions;

    BaseContestLog *loadSession(QString sessName);

    void preloadLists( );
    void preloadFiles( const QString &conarg );
    void addListSlot(QWidget *p, const QString &fname, int slotno , bool preload);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void updateLayoutsMenu();



private slots:
    void CancelClick();
    void HelpActionExecute();
    void HelpAboutActionExecute();

    void selectLayout();
    void selectSession();
    void openRecentFile();
    void FileOpenActionExecute();
    void FileImportActionExecute();
    void ManageListsActionExecute();
    void FileCloseActionExecute();
    void CloseAllActionExecute();
    void CloseAllButActionExecute();
    void OptionsActionExecute();
    void ExitClearActionExecute();
    void ExitActionExecute();

    void sessionManageExecute();

    void MakeEntryActionExecute();
    void AppendAdifActionExecute();
    void onSetMemoryActionExecute();
    void FileNewActionExecute();
    void ShiftTabLeftActionExecute();
    void ShiftTabRightActionExecute();

    void LocCalcActionExecute();
    void AnalyseMinosLogActionExecute();
    void CorrectDateTimeActionExecute();
    void AdvancedOptionsActionExecute();

    void GoToSerialActionExecute();
    void NextUnfilledActionExecute();

    void KeyerToneActionExecute();
    void KeyerTwoToneActionExecute();
    void KeyerStopActionExecute();
    void KeyerRecordActionExecute();
    void KeyerPlaybackActionExecute();

    void menuLogsActionExecute();

    void StartConfigActionExecute();

    void on_contestPageControl_currentChanged(int index);
    void onTabClosebutton(int);

    void on_TimeDisplayTimer( );
    void on_ReportOverstrike(bool , BaseContestLog * );
    void onTabMoved(int, int);
    void mleSetMemoryAction(BaseContestLog *, QString call, QString loc);
    void doScreenConfigAction();

    void appStarted();

public slots:
    void onArgsReceived(QString conarg);
    void ListOpenActionExecute();
    void ContestDetailsActionExecute();

signals:
    void sendKeyerPlay( int fno );
    void sendKeyerRecord( int fno );
    void sendKeyerTone();
    void sendKeyerTwoTone();
    void sendKeyerStop();

    void setAuxWindows();

    void logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag>);



};
extern TLogContainer *LogContainer;

#endif // TLOGCONTAINER_H

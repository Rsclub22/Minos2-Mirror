#ifndef TLOGCONTAINER_H
#define TLOGCONTAINER_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include "helpbrowser.h"
#include "n1mmbroadcast.h"
#include "rigcontrolcommonconstants.h"
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
class QLabel;

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
    friend class TAboutBox;
public:
    explicit TLogContainer(QWidget *parent = nullptr);
    ~TLogContainer() override;

    TSendDM *sendDM;
    N1MMBroadcast *n1mmBroadcast = nullptr;

    bool show(int argc, char *argv[]);
    TSingleLogFrame *getCurrentLogFrame();

    QLabel *sblabel0;
    QLabel *sblabel1;
    QLabel *sblabel2;

    QMenu TabPopup;

    SerialTVSwitch *serialTVSw = nullptr;

    TSingleLogFrame *findContest( const QString &pubname );
    TSingleLogFrame *findContest(BaseContestLog *ct );
    QVector<TSingleLogFrame *> getLogFrames();
    int getSlotNo(TSingleLogFrame *) const;

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
    void selectSession(QString sessName);

    void selectLayout(QString layout);

    void doListOpenActionExecute(QWidget *p);
    void setMenuLog(int current);
    void selectContest(BaseContestLog *pc);
    void selectTab(int t);

    void openSerialTVSwitch();
    
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

    QSharedPointer<HelpBrowser>  helpBrowser;

    static bool loggerClosing;

    void enableActions();

    void setCurrentFile(const QString &fileName);
    void removeCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    BaseContestLog * addSlot(ContestDetails *ced, const QString &fname, bool newfile, int slotno , bool hf);
    void closeSlot(int t, bool addToMRU );
    TSingleLogFrame *findLogFrame(int t);

    QAction *lastSessionSelected = nullptr;
    QAction *lastLayoutSelected = nullptr;
    QAction *lastLanguageSelected = nullptr;

    QAction *newAction(int n, QMenu *m, void (TLogContainer::*slotparam)());
    QAction *newAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)() );
    QMenu *newMenu(QMenu *m, const char *text);
    SetMemoryAction *newMemoryAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)() );
    QAction *newCheckableAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)(bool) );
    QAction *newCheckableAction(const QString text, QMenu *m, void (TLogContainer::*slotparam)(bool) );
    void setupMenus();

    void FileImportActionExecute(bool hf);
    void FileNewActionExecute(bool hf);

    QAction *EnterAction;

    QAction *HelpAction;
    QAction *HelpAboutAction;

    QAction *FileOpenAction;
    QAction *FileImportVHFAction;
    QAction *FileImportHFAction;
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
    QAction *StatsAction;
    QAction *AppendAdifAction;
    QAction *VHFFileNewAction;
    QAction *HFFileNewAction;
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
    QAction *CheckUpdatesAction;

    QAction *GoToSerialAction;
    QAction *NextUnfilledAction;

    QVector< QSharedPointer<QAction> > menuLogsActions;

    BaseContestLog *loadSession(QString sessName);

    void preloadLists( );
    void preloadFiles( const QString &conarg );
    void addListSlot(QWidget *p, const QString &fname, int slotno , bool preload);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void updateLayoutsMenu();

private slots:
    void CancelClick();
    void HelpActionExecute();
    void HelpAboutActionExecute();

    void selectLayoutAction();
    void selectSessionAction();
    void openRecentFile();
    void FileOpenActionExecute();
    void FileImportVHFActionExecute();
    void FileImportHFActionExecute();

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
    void VHFFileNewActionExecute();
    void HFFileNewActionExecute();
    void ShiftTabLeftActionExecute();
    void ShiftTabRightActionExecute();

    void LocCalcActionExecute();
    void AnalyseMinosLogActionExecute();
    void CorrectDateTimeActionExecute();
    void AdvancedOptionsActionExecute();
    void CheckUpdatesActionExecute();

    void GoToSerialActionExecute();
    void NextUnfilledActionExecute();

    void EnterActionExecute();

    void menuLogsActionExecute(bool);

    void ManageAppConfigsActionExecute();

    void on_contestPageControl_currentChanged(int index);
    void onTabClosebutton(int);

    void on_TimeDisplayTimer( );
    void on_ReportOverstrike(bool , BaseContestLog * );
    void onTabMoved(int, int);
    void mleSetMemoryAction(BaseContestLog *, QString call, QString loc);
    void doScreenConfigAction();

    void stealFocus();
    void StatsActionExecute();
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

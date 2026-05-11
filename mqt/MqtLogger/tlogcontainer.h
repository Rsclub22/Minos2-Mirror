#ifndef TLOGCONTAINER_H
#define TLOGCONTAINER_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include "loggerscreenoptions.h"
#include "n1mmbroadcast.h"
#include "rigcontrolcommonconstants.h"
#include "serialtvswitch.h"

namespace Ui {
class TLogContainer;
}

class ContestDetails;
class BaseContestLog;
class LoggerContestLog;
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

    LoggerScreenOptions loggerScreenOptions;
    TSendDM *sendDM;
    N1MMBroadcast *n1mmBroadcast = nullptr;

    bool show(int argc, char *argv[]);
    TSingleLogFrame *getCurrentLogFrame();

    QLabel *sblabel0;
    QLabel *sblabel1;
    QLabel *sblabel2;

    QMenu TabPopup;

    SerialTVSwitch *serialTVSw = nullptr;

    QString aboutBoxOpenFilename;

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
    
    QStringList createContest(bool hf);
    void showContest(LoggerContestLog* contest, int slotNo);

private:
    Ui::TLogContainer *ui;

    QTimer TimerUpdateQSOTimer;

    QMap<QAction *, const char *> actionList;
    QMap<QMenu *, const char *> menuList;

    QMenu *screenLayoutMenu = nullptr;
    enum { MaxRecentFiles = 5 };
    QVector<QAction *> recentFileActs;
    QMenu *recentFilesMenu = nullptr;
    QVector<QAction *> sessionActs;
    QMenu *sessionsMenu = nullptr;

    static bool loggerClosing;

    void enableActions();

    void setCurrentFile(const QString &fileName);
    void removeCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    BaseContestLog * addSlot(ContestDetails *ced, const QString &fname, int slotno , bool hf);
    void closeSlot(int t, bool addToMRU );
    TSingleLogFrame *findLogFrame(int t);

    QAction *lastSessionSelected = nullptr;
    QAction *lastLayoutSelected = nullptr;
    QAction *lastLanguageSelected = nullptr;

    QAction *newAction(int n, QMenu *m, void (TLogContainer::*slotparam)(),QAction::MenuRole mr = QAction::TextHeuristicRole);
    QAction *newAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)() ,QAction::MenuRole mr = QAction::TextHeuristicRole);
    QMenu *newMenu(QMenu *m, const char *text);
    SetMemoryAction *newMemoryAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)() );
    QAction *newCheckableAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)(bool) );
    QAction *newCheckableAction(const QString text, QMenu *m, void (TLogContainer::*slotparam)(bool) );
    void setupMenus();
    void clearMenus();

    void FileImportActionExecute(bool hf);
    QStringList FileNewActionExecute(bool hf, bool fromAbout);

    QAction *EnterAction = nullptr;

    QAction *HelpAction;
    QAction *HelpAboutAction;

    QAction *FileOpenAction = nullptr;
    QAction *FileImportVHFAction = nullptr;
    QAction *FileImportHFAction = nullptr;
    QAction *ListOpenAccept = nullptr;
    QAction *ContestDetailsAction = nullptr;
    QAction *FileCloseAction = nullptr;
    QAction *CloseAllAction = nullptr;
    QAction *CloseAllButAction = nullptr;
    QAction *OptionsAction = nullptr;

    QAction *sessionManagerAction = nullptr;

    QAction *ExitAction = nullptr;
    QAction *ExitClearAction = nullptr;
    QAction *MakeEntryAction = nullptr;
    QAction *StatsAction = nullptr;
    QAction *AppendAdifAction = nullptr;
    QAction *VHFFileNewAction = nullptr;
    QAction *HFFileNewAction = nullptr;
    QAction *ListOpenAction = nullptr;
    QAction *ManageListsAction = nullptr;
    QAction *ShiftTabLeftAction = nullptr;
    QAction *ShiftTabRightAction = nullptr;

    QAction *startConfigAction = nullptr;
    QAction *LocCalcAction = nullptr;
    QAction *AnalyseMinosLogAction = nullptr;
    QAction *CorrectDateTimeAction = nullptr;
    QAction *ManageHamlibAction = nullptr;
    QAction *DownloadFilesAction = nullptr;
    QAction* manageSpotDatabaseAction = nullptr;
    QAction *ScreenConfigAction = nullptr;
    QAction *AdvancedOptionsAction = nullptr;
    QAction *CheckUpdatesAction = nullptr;

    QAction *GoToSerialAction = nullptr;
    QAction *NextUnfilledAction = nullptr;

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

    bool inspectGeometry(const QByteArray &geometry);
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
    void ManageAdifActionExecute();
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
    void ManageHamlibActionExecute();
    void on_manageSpotsDatabaseActionSelected();
    void on_downloadFilesActionSelected();
    void onScreenConfigApply(QString curConfigName);
    void onSetDefaultName(QString def);
    void onSetProtectedName(QString prot);
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

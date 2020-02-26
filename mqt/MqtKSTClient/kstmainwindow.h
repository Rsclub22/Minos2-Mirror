#ifndef KSTMAINWINDOW_H
#define KSTMAINWINDOW_H

#include "base_pch.h"

#include "airscoutlink.h"

#include <QMainWindow>
#include <QTcpSocket>
#include <QRadioButton>
#include "kstcallgridmodel.h"
#include "kstmessagegridmodel.h"
#include "kstplanesmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class KSTMainWindow; }
QT_END_NAMESPACE

extern QStringList services;

class KSTMainWindow : public QMainWindow
{
    Q_OBJECT

    QTimer CloseTimer;
    QTimer userCallTimer;

    KstMessageGridModel kstMessageModel;
    KstMessageGridSortFilterModel kstMessageFilterModel;

    KstMeepGridSortFilterModel kstMeepFilterModel;

    QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > messageVector;

    KstCallGridModel kstCallModel;
    KstCallGridSortFilterModel kstCallFilterModel;

    QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector;
    bool callVectorChanged = false;

    KstPlanesModel kstPlanesModel;
    KstPlanesGridSortFilterModel kstPlanesFilterModel;

    QSharedPointer<HtmlDelegate> meepDelegate;
    QSharedPointer<HtmlDelegate> messageDelegate;
    QSharedPointer<HtmlDelegate> CSDelegate;
    QSharedPointer<HtmlDelegate> PlanesDelegate;

    QTcpSocket* kstclient;

    QString serverName;
    QString serverPort;
    QString myCallsign;
    QString password;
    QVector<int> kstChatSelection;
    QVector<int> kstLoggedIn;
    int activeChat = 1;

    QString myLoc;
    bool autoConnect = false;

    int maxDistance = 99999;

    bool ASActive = false;
    ASBand ASActiveBand = asb144M;
    QString ASServerName = "AS";
    QString ASMyName = "Minos";
    int ASMinDistance = 300;
    int ASMaxDistance = 1000;

    bool kstconnected = false;
    bool started = false;

    QString msgbuf;
    QStringList filelines;
    int curline = 0;

    int messageChatFilter = 0;
    int CSChatFilter = 0;

    int messageSequence = 0;

    QSharedPointer<AirScoutLink> asl;
    QSharedPointer<KstUser> planeActive;

    void sendKST(QString msg);
    void analyseKstMessage(QString atj);
    void reconnect();
    void connectToHost();
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    void setNameFromCall(QString call);
    void doLoginChanges();
    void setActive(int chat);
    bool doConfiguration();

public:
    KSTMainWindow(QWidget *parent = nullptr);
    ~KSTMainWindow() override;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    int getMaxDistance() const;

    bool getASActive() const;

    ASBand getASActiveBand() const;

    QString getASServerName() const;

    QString getASMyName() const;

    int getASMinDistance() const;

    int calcDistance(QString c);

    int getASMaxDistance() const;

    QString getMyCallsign() const;

    QString getMyLoc() const;

    QSharedPointer<QVector<QSharedPointer<KstUser> > > getCallVector() const;

    int getActiveChat() const;

    void showPlanes(QSharedPointer<KstUser> user);

    QSharedPointer<KstUser> getUser(QString call);
private slots:
    void CloseTimerTimer();

    void userCallTimerTimer();

    void on_analyseButton_clicked();

    void on_closeButton_clicked();

    void on_connectButton_clicked();

    void on_messageFilter_textChanged(const QString &arg1);

    void on_CSFilter_textChanged(const QString &arg1);

    void on_kstSplitter_splitterMoved(int pos, int index);

    void on_sectionResized(int, int, int);
    void on_CSTable_clicked(const QModelIndex &index);

    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);
    void onReadyRead();

    void on_configureButton_clicked();

    void on_genmsgButton_clicked();

    void on_meepButton_clicked();

    void on_msgSplitter_splitterMoved(int pos, int index);

    void on_messageTable_clicked(const QModelIndex &index);

    void on_meepTable_clicked(const QModelIndex &index);

    void on_clearButton_clicked();

    void on_sortIndicatorChanged(int, Qt::SortOrder);
    void on_callEdit_textChanged(const QString &arg1);

    void on_clearMessageButton_clicked();

    void on_awayButton_clicked();

    void logincb_stateChanged(int arg1);
    void activerb_clicked();
    void on_CSChatFilter_currentIndexChanged(int index);
    void on_messageChatFilter_currentIndexChanged(int index);

    void on_clearMessageFilter_clicked();

    void on_clearUserFilter_clicked();

    void on_sectionMoved(int, int, int);

    void acChanged(QSharedPointer<KstUser>);

    void on_asBandCombo_currentIndexChanged(int index);

    void on_ASActivecb_stateChanged(int arg1);

    void on_showInAS_clicked();

    void on_showMPath_clicked();

    void on_callSplitter_splitterMoved(int pos, int index);

private:
    Ui::KSTMainWindow *ui;
    void clearConnection();
    void checkActive();
    void resetVectors(QCheckBox *cb, QRadioButton *rb, int c, QStringList &s, QVector<int> &v, QVector<int> &a);
    void checkAwayButton();
};

extern KSTMainWindow *mainWindow;
#endif // KSTMAINWINDOW_H

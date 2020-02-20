#ifndef KSTMAINWINDOW_H
#define KSTMAINWINDOW_H

#include "base_pch.h"

#include "airscoutlink.h"

#include <QMainWindow>
#include <QTcpSocket>
#include <QRadioButton>
#include "kstcallgridmodel.h"
#include "kstmessagegridmodel.h"

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


    QSharedPointer<HtmlDelegate> meepDelegate;
    QSharedPointer<HtmlDelegate> messageDelegate;
    QSharedPointer<HtmlDelegate> CSDelegate;

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

    bool kstconnected = false;
    bool started = false;

    QString msgbuf;
    QStringList filelines;
    int curline = 0;

    int messageChatFilter = 0;
    int CSChatFilter = 0;

    int messageSequence = 0;

    QSharedPointer<AirScoutLink> asl;

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
private:
    Ui::KSTMainWindow *ui;
    void clearConnection();
    void checkActive();
    void resetVectors(QCheckBox *cb, QRadioButton *rb, int c, QStringList &s, QVector<int> &v, QVector<int> &a);
    void checkAwayButton();
};
#endif // KSTMAINWINDOW_H

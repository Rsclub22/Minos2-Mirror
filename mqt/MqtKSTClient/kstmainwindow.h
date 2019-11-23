#ifndef KSTMAINWINDOW_H
#define KSTMAINWINDOW_H

#include "base_pch.h"

#include <QMainWindow>
#include "qttelnet.h"
#include "kstcallgridmodel.h"
#include "kstmessagegridmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class KSTMainWindow; }
QT_END_NAMESPACE

class KSTMainWindow : public QMainWindow
{
    Q_OBJECT

    KstMessageGridModel kstMessageModel;
    KstMessageGridSortFilterModel kstMessageFilterModel;

    KstMessageGridModel kstMeepModel;
    KstMessageGridSortFilterModel kstMeepFilterModel;

    QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > messageVector;

    KstCallGridModel kstCallModel;
    KstCallGridSortFilterModel kstCallFilterModel;

    QSharedPointer<QStringList > callVector;


    QSharedPointer<HtmlDelegate> meepDelegate;
    QSharedPointer<HtmlDelegate> messageDelegate;
    QSharedPointer<HtmlDelegate> CSDelegate;

    QtTelnet* tnclient;

    QString serverName;
    QString serverPort;
    QString callsign;
    QString password;
    QString kstChatSelection;
    bool autoConnect = false;

    QString msgbuf;
    QStringList filelines;
    int curline = 0;

    bool userLoggedIn = false;
    bool setupComplete = false;

public:
    KSTMainWindow(QWidget *parent = nullptr);
    ~KSTMainWindow() override;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;

private slots:

    void on_analyseButton_clicked();

    void on_closeButton_clicked();

    void on_connectButton_clicked();

    void on_messageFilter_textChanged(const QString &arg1);

    void on_CSFilter_textChanged(const QString &arg1);

    void on_kstSplitter_splitterMoved(int pos, int index);

    void on_sectionResized(int, int, int);
    void on_CSTable_clicked(const QModelIndex &index);

    void connectToHost();
    void connectionEstab();
    void connectionError(QAbstractSocket::SocketError error);
    void logIn();
    void loggedOut();
    void messageRx(QString msg);

    void on_configureButton_clicked();


    void on_genmsgButton_clicked();

    void on_meepButton_clicked();

    void on_msgSplitter_splitterMoved(int pos, int index);

    void on_messageTable_clicked(const QModelIndex &index);

    void on_meepTable_clicked(const QModelIndex &index);

    void on_serviceCombo_currentIndexChanged(int index);

    void on_clearButton_clicked();

private:
    Ui::KSTMainWindow *ui;
    void analyseTelnetMessage(QString atj);
    void reconnect();
};
#endif // KSTMAINWINDOW_H

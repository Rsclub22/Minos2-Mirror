#ifndef KSTMAINWINDOW_H
#define KSTMAINWINDOW_H

#include "base_pch.h"

#include <QMainWindow>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "qttelnet.h"
#include "htmldelegate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class KSTMainWindow; }
QT_END_NAMESPACE


class CallGridModel: public QAbstractItemModel
{
    Q_OBJECT

    public:
        CallGridModel();
        virtual ~CallGridModel() override
        {}
        QSharedPointer<QStringList> callVector;
        QSharedPointer<HtmlDelegate> delegate;

        void setCallVector(QSharedPointer<QStringList > pcallVector);
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;


        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index )const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() )const Q_DECL_OVERRIDE;

        void appendRow(QString kstmsg);
};

class CallGridSortFilterModel: public QSortFilterProxyModel
{
    QString filterString;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
};
class ChatLine
{
public:
    QString source;
    QString dtg;
    QString fullLine;
    QString call;
    QString name;
    QString otherCall;
    QString message;

    ChatLine(){}
    ~ChatLine(){}
};

class ChatGridModel: public QAbstractItemModel
{
    Q_OBJECT

    public:
        ChatGridModel();
        virtual ~ChatGridModel() override
        {}
        QSharedPointer<QVector <QSharedPointer<ChatLine> > > chatVector;
        QSharedPointer<HtmlDelegate> delegate;

        void setChatVector(QSharedPointer<QVector<QSharedPointer<ChatLine> > > pchatVector);
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;


        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index )const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() )const Q_DECL_OVERRIDE;

        void appendRow(QSharedPointer<ChatLine> kstmsg);
};

class ChatGridSortFilterModel: public QSortFilterProxyModel
{
    QString filterString;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
};

class KSTMainWindow : public QMainWindow
{
    Q_OBJECT

    ChatGridModel cgm;
    ChatGridSortFilterModel cgsfm;

    QSharedPointer<QVector <QSharedPointer<ChatLine> > > chatVector;

    CallGridModel clgm;
    CallGridSortFilterModel clgsfm;

    QSharedPointer<QStringList > callVector;


    QSharedPointer<HtmlDelegate> messageDelegate;
    QSharedPointer<HtmlDelegate> CSDelegate;

    QtTelnet* tnclient;

    QString hostname;
    QString port;
    QString username;
    QString password;
    QString service;

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

private:
    Ui::KSTMainWindow *ui;
    void analyseFileMessage(QString atj);
    void analyseTelnetMessage(QString atj);
};
#endif // KSTMAINWINDOW_H

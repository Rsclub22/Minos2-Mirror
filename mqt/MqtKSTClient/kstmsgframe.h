#ifndef KSTMSGFRAME_H
#define KSTMSGFRAME_H

#include <QFrame>

#include "kstmessagegridmodel.h"
#include "minospanel.h"

namespace Ui {
class KSTMsgFrame;
}

class KSTMsgFrame : public MinosPanel
{
    Q_OBJECT

    QSharedPointer<HtmlDelegate> messageDelegate;

    bool mouseInMessages = false;

    virtual bool eventFilter(QObject *obj, QEvent *event) override;
public:
    KstMessageGridModel kstMessageModel;
    KstMessageGridSortFilterModel kstMessageFilterModel;

    explicit KSTMsgFrame(QWidget *parent = nullptr);
    ~KSTMsgFrame();

    void on_FontChanged();

    void setServices(QStringList services);
    void scrollMesToBottom();
    void setFilter(QString s);
    void setActive(int chat);
    void setFocus();
    void showAirscoutPath();
public slots:
    void on_clearMessageFilter_clicked();
private slots:
    void on_sectionResized(int, int, int);
    void on_sectionMoved(int, int, int);
    void on_messageTable_doubleClicked(const QModelIndex &index);
    void on_messageChatFilter_currentIndexChanged(int index);
    void on_messageTable_clicked(const QModelIndex &index);
    void on_clearSelectedMessage_clicked();
    void on_messageFilter_textChanged(const QString &arg1);
private:
    Ui::KSTMsgFrame *ui;
};

#endif // KSTMSGFRAME_H

#ifndef KSTTOMEFRAME_H
#define KSTTOMEFRAME_H

#include "kstmessagegridmodel.h"
#include <QFrame>

namespace Ui {
class KSTTomeFrame;
}

class KSTTomeFrame : public QFrame
{
    Q_OBJECT

    QSharedPointer<HtmlDelegate> meepDelegate;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

public:
    KstMeepGridSortFilterModel kstMeepFilterModel;

    explicit KSTTomeFrame(QWidget *parent = nullptr);
    ~KSTTomeFrame();

    void on_FontChanged();

    void scrollMeepToBottom();
    void setMeepFilters();
    void setConnected(bool c);
private slots:
    void on_sectionResized(int, int, int);
    void on_sectionMoved(int, int, int);
    void on_meepTable_doubleClicked(const QModelIndex &index);
    void on_includeMeCb_stateChanged(int);
    void on_toMeFilter_textChanged(const QString &);
    void on_clearMeepFiltersButton_clicked();
    void on_showReadcb_stateChanged(int);
    void on_meepTable_clicked(const QModelIndex &index);
    void on_toFromMecb_stateChanged(int arg1);

private:
    Ui::KSTTomeFrame *ui;
};

#endif // KSTTOMEFRAME_H

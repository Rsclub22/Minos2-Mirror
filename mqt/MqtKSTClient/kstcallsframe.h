#ifndef KSTCALLSFRAME_H
#define KSTCALLSFRAME_H

#include <QFrame>

#include "kstcallgridmodel.h"
#include "minospanel.h"

namespace Ui {
class KSTCallsFrame;
}

class KSTCallsFrame : public MinosPanel
{
    Q_OBJECT

    QSharedPointer<HtmlDelegate> CSDelegate;

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

public:
    KstCallGridModel kstCallModel;
    KstCallGridSortFilterModel kstCallFilterModel;

    explicit KSTCallsFrame(QWidget *parent = nullptr);
    ~KSTCallsFrame();

    void on_FontChanged();

    void setServices(QStringList services);

    void acChanged(QSharedPointer<KstUser> user);
    void setDefaultButton(bool s);
    void setFilter(Callsign &c);
    void showAircout(bool s);
private slots:
    void on_sectionResized(int, int, int);
    void on_sectionMoved(int, int, int);
    void on_sortIndicatorChanged(int, Qt::SortOrder);
    void onCSTableSelectionChanged(const QItemSelection &, const QItemSelection &);
    void on_loggerXferButton_clicked();

    void on_stringRb_clicked();
    void on_countryRb_clicked();
    void on_clearUserFilter_clicked();
    void on_CSFilter_textChanged(const QString &arg1);
    void on_CSChatFilter_currentIndexChanged(int index);
    void on_awayCallscb_stateChanged(int);
    void on_inactiveCallscb_stateChanged(int);
    void on_maxDistanceEdit_editingFinished();
    void on_workedCallscb_stateChanged(int);

private:
    Ui::KSTCallsFrame *ui;
};

#endif // KSTCALLSFRAME_H

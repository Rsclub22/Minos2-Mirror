#ifndef KSTPLANESFRAME_H
#define KSTPLANESFRAME_H

#include <QFrame>

#include "htmldelegate.h"
#include "kstplanesmodel.h"
#include "minospanel.h"

namespace Ui {
class KSTPlanesFrame;
}

class KSTPlanesFrame : public MinosPanel
{
    Q_OBJECT

    QSharedPointer<HtmlDelegate> PlanesDelegate;
    QSharedPointer<KstUser> planeActive;
public:
    KstPlanesModel kstPlanesModel;
    KstPlanesGridSortFilterModel kstPlanesFilterModel;

    explicit KSTPlanesFrame(QWidget *parent = nullptr);
    ~KSTPlanesFrame();

    void on_FontChanged();

    void showPlanes(QSharedPointer<KstUser> user);
private slots:
    void on_sectionResized(int, int, int);
    void on_sectionMoved(int, int, int);
    void on_showInAS_clicked();
    void on_showMPath_clicked();
    void onListCompressionChanged(qreal);
public slots:
    void acChanged(QSharedPointer<KstUser> user);
private:
    Ui::KSTPlanesFrame *ui;
};

#endif // KSTPLANESFRAME_H

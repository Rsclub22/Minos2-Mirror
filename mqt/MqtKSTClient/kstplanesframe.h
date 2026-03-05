#ifndef KSTPLANESFRAME_H
#define KSTPLANESFRAME_H

#include <QFrame>

#include "htmldelegate.h"
#include "kstplanesmodel.h"

namespace Ui {
class KSTPlanesFrame;
}

class KSTPlanesFrame : public QFrame
{
    Q_OBJECT

 //   virtual bool eventFilter(QObject *obj, QEvent *event) override;

    QSharedPointer<HtmlDelegate> PlanesDelegate;
    QSharedPointer<KstUser> planeActive;
    KstPlanesModel *kstPlanesModel = nullptr;
public:
    explicit KSTPlanesFrame(QWidget *parent = nullptr);
    ~KSTPlanesFrame();

    void on_FontChanged();

    void setModel(KstPlanesModel &kstPlanesModel, KstPlanesGridSortFilterModel &kstPlanesFilterModel);


    void showPlanes(QSharedPointer<KstUser> user);
private slots:
    void on_sectionResized(int, int, int);
    void on_sectionMoved(int, int, int);
    void on_showInAS_clicked();
    void on_showMPath_clicked();
public slots:
    void acChanged(QSharedPointer<KstUser> user);
private:
    Ui::KSTPlanesFrame *ui;
};

#endif // KSTPLANESFRAME_H

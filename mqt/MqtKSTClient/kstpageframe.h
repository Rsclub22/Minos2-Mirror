#ifndef KSTPAGEFRAME_H
#define KSTPAGEFRAME_H

#include "ScreenConfigFile.h"
#include <QFrame>

namespace Ui {
class KSTPageFrame;
}
class QVBoxLayout;
class MinosSplitter;

class KSTPageFrame : public QFrame
{
    Q_OBJECT

private:
    Ui::KSTPageFrame *ui;


protected:
    QVBoxLayout *verticalLayout = nullptr;

public:
    explicit KSTPageFrame(QWidget *parent, int pno);
    virtual ~KSTPageFrame();

    int pageNo = -1;
    QString pageName;
    QVector <MinosSplitter *> rowSplitters;

    void buildScreen(SCScreen &s);
    void clearScreen();
    bool hasElements();
private:
    bool splittersChanged;
    int splitterHandleWidth;
    MinosSplitter *kstPageSplitter = nullptr;

    void getSplitters();
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
private slots:
    void onSplittersChanged();
    void onSplitterMoved(int, int);
    void on_doSplitterChanges();

    void onCustomContextMenuRequested(const QPoint &pos);
};

#endif // KSTPAGEFRAME_H

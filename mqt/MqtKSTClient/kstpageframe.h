#ifndef KSTPAGEFRAME_H
#define KSTPAGEFRAME_H

#include "ScreenConfigFile.h"
#include <QFrame>

namespace Ui {
class KSTPageFrame;
}
class QVBoxLayout;
class MinosSplitter;
class KSTMainFrame;

class KSTPageFrame : public QFrame
{
    Q_OBJECT

private:
    Ui::KSTPageFrame *ui;


protected:
    QVBoxLayout *verticalLayout = nullptr;

public:
    explicit KSTPageFrame(QWidget *parent);
    virtual ~KSTPageFrame();

    int pageNo = -1;
    QString pageName;
    QVector <MinosSplitter *> rowSplitters;

    void buildScreen(KSTMainFrame *tslf, SCScreen &s);
    void clearScreen();
    bool hasElements();
private:
    bool splittersChanged;
    int splitterHandleWidth;
    MinosSplitter *kstPageSplitter = nullptr;


    KSTMainFrame *tslf = nullptr;
    void getSplitters();

private slots:
    void onSplittersChanged();
    void onSplitterMoved(int, int);
    void on_doSplitterChanges();

};

#endif // KSTPAGEFRAME_H

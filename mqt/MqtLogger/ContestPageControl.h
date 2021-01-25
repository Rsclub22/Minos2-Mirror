#ifndef CONTESTPAGECONTROL_H
#define CONTESTPAGECONTROL_H

#include "qlogtabwidget.h"

class BaseContestLog;
class ContestPage;

namespace Ui {
class ContestPageControl;
}

class ContestPageControl : public QLogTabWidget
{
    Q_OBJECT

public:
    explicit ContestPageControl(QWidget *parent = nullptr);
    ~ContestPageControl();

    int getInstance() const;
    void setInstance(int value);

    QMap<BaseContestLog *, ContestPage *> pages;
private:
    Ui::ContestPageControl *ui;
    int instance = 0;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

private slots:
    void onContestShownChanged();

    void onTabBarClicked(int index);

    void onCustomContextMenuRequested(const QPoint &pos);

    void onTabBarDoubleClicked(int);
};

#endif // CONTESTPAGECONTROL_H

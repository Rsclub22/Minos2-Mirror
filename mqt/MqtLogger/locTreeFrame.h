#ifndef LOCTREEFRAME_H
#define LOCTREEFRAME_H

#include "base_pch.h"

namespace Ui {
class LocTreeFrame;
}

class BaseContestLog;
class LocCount;


class LocTreeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LocTreeFrame(QWidget *parent = nullptr);
    ~LocTreeFrame();

    void reInitialiseLocators();
    void setContest(BaseContestLog *contest);

private:
    Ui::LocTreeFrame *ui;
    QSharedPointer<HtmlDelegate> delegate;

    BaseContestLog *ct;
};

#endif // LOCTREEFRAME_H

#ifndef LOCTREEFRAME_H
#define LOCTREEFRAME_H

#include <QFrame>
#include "htmldelegate.h"

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
    void setBand(QString band);

private:
    Ui::LocTreeFrame *ui;
    QSharedPointer<HtmlDelegate> delegate;

    BaseContestLog *ct;
    QString band;
};

#endif // LOCTREEFRAME_H

#ifndef CONTESTPAGE_H
#define CONTESTPAGE_H

#include "base_pch.h"

class TSingleLogFrame;
class TMatchCollection;
class MatchNodeData;
class MatchTreeItem;
class ProtoContest;
class BaseContestLog;
class BaseContact;
class ContactList;
class ListContact;
class FocusWatcher;
class MatchTreeFrame;
class ChatFrame;
class ClusterClientFrame;
class BandmapClientFrame;
class RigControlFrame;
class RunButtonsFrame;
class RotControlFrame;
class RotPresets;
class QSOLogFrame;
class MatchThisFrame;
class MatchOtherFrame;
class MatchArchiveFrame;
class WsjtxFrame;

class SCScreen;
class SCRow;

class MinosSplitter;

namespace Ui {
class ContestPage;
}

// this is all the per window instance stuff
// overall action is in TSingleLogFrame

class ContestPage : public QFrame
{
    friend class TSendDM;
    Q_OBJECT

protected:
    QVBoxLayout *verticalLayout = nullptr;

    QVector <MinosSplitter *> rowSplitters;
    BaseContestLog *contest = nullptr;

public:
    explicit ContestPage(QWidget *parent, BaseContestLog *ct);
    ~ContestPage();

    int pageNo = -1;
    QString pageName;

    BaseContestLog * getContest();

    void buildScreen(TSingleLogFrame *tslf, SCScreen &s, int auxInstance);
    void clearScreen();
private:
    Ui::ContestPage *ui;
    bool splittersChanged;
    int splitterHandleWidth;

    MinosSplitter *singleLogFrameSplitter = nullptr;

    TSingleLogFrame *tslf = nullptr;
    void getSplitters();
 private slots:
    void onSplittersChanged();
    void onSplitterMoved(int, int);
    void on_doSplitterChanges(BaseContestLog*);
    void on_ContestShownChanged();

};

#endif // CONTESTPAGE_H

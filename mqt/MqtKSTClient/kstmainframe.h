#ifndef KSTMAINFRAME_H
#define KSTMAINFRAME_H

#include <QFrame>

#include "ScreenConfigFile.h"
#include "kstpageframe.h"

namespace Ui {
class KSTMainFrame;
}
class KSTActiveChatsFrame;
class KSTButtonsFrame;
class KSTCallsFrame;
class KSTLoginFrame;
class KSTMsgFrame;
class KSTPlanesFrame;
class KSTSendMeepFrame;
class KSTTomeFrame;

class KSTMainFrame : public KSTPageFrame
{
    Q_OBJECT

public:
    explicit KSTMainFrame(QWidget *parent = nullptr);
    ~KSTMainFrame();

    static bool inApplyScreenLayout;

    void createScreenComponents();

    void buildRow(KSTPageFrame *cp, SCRow &scrow, MinosSplitter *splitterParent);
    QString getCurScreenLayout() const;
    void applyScreenLayout();
    void setCurScreenLayout(const QString &value);

    QVector<KSTPageFrame *> pages;

    KSTActiveChatsFrame *kstActiveChatsFrame = nullptr;
    KSTButtonsFrame *kstButtonsFrame = nullptr;
    KSTCallsFrame *kstCallsFrame = nullptr;
    KSTLoginFrame *kstLoginFrame = nullptr;
    KSTMsgFrame *kstMsgFrame = nullptr;
    KSTPlanesFrame *kstPlanesFrame = nullptr;
    KSTSendMeepFrame *kstSendMeepFrame = nullptr;
    KSTTomeFrame *kstTomeFrame = nullptr;

private:
    Ui::KSTMainFrame *ui;
    QString curScreenLayout;

    void traceMsg(QString msg);

    void buildScreenLayout();
    void clearScreenLayout(bool clearAllTabs);
    void buildScreen(SCScreen &s, int t);


};

#endif // KSTMAINFRAME_H

#ifndef SCREENCONFIGELEMENT_H
#define SCREENCONFIGELEMENT_H

#include <QSharedPointer>
#include <QFrame>
//#include "StackedInfoFrame.h"
#include "ScreenConfigFile.h"
#include "ScreenConfig.h"
#include "auxentries.h"

class ScreenConfigScreen;
class ScreenConfigRow;
class SCTypeOption
{
public:
    SCType type;
    const char * s;
    const char * hint;
};

namespace Ui {
class ScreenConfigElement;
}

class ScreenConfigElement : public QFrame
{
    Q_OBJECT
    static  QVector <SCTypeOption> scoptions;
public:
    Ui::ScreenConfigElement *ui;
    QVBoxLayout *vbl = nullptr;

    explicit ScreenConfigElement(ScreenConfigRow *parentrow, ScreenConfigScreen *sc = nullptr);
    ~ScreenConfigElement();

    void setType(SCType);
    SCType getType() const;

    void setAuxType(AuxEntryType);
    AuxEntryType getAuxType() const;

    bool getIsSplitElement() const;
    void setIsSplitElement(bool value);

    void addRowBefore(ScreenConfigRow *r);
    void removeRow(ScreenConfigRow *r);
    void addRowAfter(ScreenConfigRow *r);

    bool checkOk(ScreenConfigElement *e);

    static SCType getScreenType(QString s);
    static QString getTrScreenHint(SCType s);
    static const char *getRawScreenTypeString(SCType t);
    static const char *getRawScreenHint(SCType t);
    static void setScreenOptions(QVector<SCTypeOption> &sco);
private slots:
    void on_elementTypeCombo_activated(int arg1);

    void on_addLeftButton_clicked();

    void on_addRightButton_clicked();

    void on_removeButton_clicked();

    void on_splitAboveButton_clicked();

    void on_splitBelowButton_clicked();

private:
    ScreenConfigRow *parentRow = nullptr;
    ScreenConfigScreen *parentDialog = nullptr;
    bool isSplitElement = false;

};

#endif // SCREENCONFIGELEMENT_H

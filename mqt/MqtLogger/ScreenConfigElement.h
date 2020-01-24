#ifndef SCREENCONFIGELEMENT_H
#define SCREENCONFIGELEMENT_H

#include "StackedInfoFrame.h"
#include "ScreenConfigFile.h"
#include "ScreenConfig.h"

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
    static QVector <SCTypeOption> scoptions;
public:
    Ui::ScreenConfigElement *ui;
    QVBoxLayout *vbl = nullptr;

    explicit ScreenConfigElement(ScreenConfigRow *parentrow, ScreenConfig *sc = nullptr);
    ~ScreenConfigElement();

    void setType(SCType);
    QString getType() const;

    void setAuxType(AuxEntries);
    QString getAuxType() const;

    bool getIsSplitElement() const;
    void setIsSplitElement(bool value);

    void eraseLayout(QLayout *layout);

    void addRowBefore(ScreenConfigRow *r);
    void removeRow(ScreenConfigRow *r);
    void addRowAfter(ScreenConfigRow *r);

    bool checkOk(ScreenConfigElement *e);

    static SCType getScreenType(QString s);
    static QString getScreenTypeString(SCType s);
private slots:
    void on_elementTypeCombo_activated(const QString &arg1);

    void on_addLeftButton_clicked();

    void on_addRightButton_clicked();

    void on_removeButton_clicked();

    void on_splitAboveButton_clicked();

    void on_splitBelowButton_clicked();

private:
    ScreenConfigRow *parentRow = nullptr;
    ScreenConfig *parentDialog = nullptr;
    bool isSplitElement = false;

};

#endif // SCREENCONFIGELEMENT_H

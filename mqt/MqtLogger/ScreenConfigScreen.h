#ifndef SCREENCONFIGSCREEN_H
#define SCREENCONFIGSCREEN_H

#include <QFrame>
#include <QVBoxLayout>
#include "ScreenConfigFile.h"

class ScreenConfig;
class ScreenConfigScreen;
class ScreenConfigRow;
class ScreenConfigElement;
class ScreenConfigAddColumn;

namespace Ui {
class ScreenConfigScreen;
}

class ScreenConfigScreen : public QFrame
{
    Q_OBJECT

public:
    explicit ScreenConfigScreen(ScreenConfig *parentc);
    ~ScreenConfigScreen();
    QVBoxLayout *vbl = nullptr;
    ScreenConfig *parentElement = nullptr;
    ScreenConfigElement *baseElement = nullptr;

    bool mainScreen = false;
    QString name;

    void setName(QString n);
    bool checkOk(ScreenConfigElement *e);

    void checkAddButtons();
    void addColumnLeft(ScreenConfigElement * e, int top, int bottom);
    void addColumnRight(ScreenConfigElement * e, int top, int bottom);
    void procRowSel(ScreenConfigRow *row, QVector<ScreenConfigRow *> &sel);
    QVector<ScreenConfigRow *> getSelected();
    void buildRows(QVector<SCRow> rows, ScreenConfigElement *bele, QVBoxLayout *vbl);
    void getConfig(SCScreen &sc);
    bool checkScreenOk(ScreenConfigElement *e, int auxCount);
public slots:
    void on_addRowButton_clicked();

    void on_addColumnRightButton_clicked();

    void on_addColumnLeftButton_clicked();
private slots:
    void on_screenNameEdit_textEdited(const QString &arg1);

private:
    Ui::ScreenConfigScreen *ui;

    void procRow(ScreenConfigRow *row, SCRow &scrow);
    bool checkRowOk(const ScreenConfigRow *row, ScreenConfigElement *e, int &auxCount);
    ScreenConfigRow *combineRows(ScreenConfigElement * e, int top, int bottom);
    int getTopRow(ScreenConfigElement * e);
    int getBottomRow(ScreenConfigElement *e);

};

#endif // SCREENCONFIGSCREEN_H

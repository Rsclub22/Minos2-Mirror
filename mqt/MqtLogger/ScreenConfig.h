#ifndef SCREENCONFIG_H
#define SCREENCONFIG_H

#include <QDialog>
#include <QVBoxLayout>

#include "ScreenConfigFile.h"

class ScreenConfigRow;
class ScreenConfigElement;
class ScreenConfigAddColumn;

namespace Ui {
class ScreenConfig;
}

class ScreenConfig : public QDialog
{
    Q_OBJECT

public:
    QVBoxLayout *vbl;

    explicit ScreenConfig(QWidget *parent, ScreenConfigFile &scf, QString curConfigName);
    ~ScreenConfig() override;

    ScreenConfigElement *baseElement = nullptr;

    int topRowCount();

    bool checkOk(ScreenConfigElement *e);

    void checkAddRowButton();
    void addColumnLeft(ScreenConfigElement * e, int top, int bottom);
    void addColumnRight(ScreenConfigElement * e, int top, int bottom);
    void procRowSel(ScreenConfigRow *row, QVector<ScreenConfigRow *> &sel);
    QVector<ScreenConfigRow *> getSelected();
public slots:
    void reject() override;
    void accept() override;
private slots:
    void on_OKButton_clicked();

    void on_applyButton_clicked();

    void on_cancelButton_clicked();

    void on_addRowButton_clicked();

    void on_addColumnRightButton_clicked();

    void on_addColumnLeftButton_clicked();

private:
    Ui::ScreenConfig *ui;

    ScreenConfigFile &scf;
    QString curConfigName;

    void doCloseEvent();
    SC getConfig();
    void buildRows(QVector<SCRow> rows, ScreenConfigElement *bele, QVBoxLayout *vbl);
    void procRow(ScreenConfigRow *row, SCRow &scrow);
    bool checkRowOk(const ScreenConfigRow *row, ScreenConfigElement *e, int &auxCount);
    ScreenConfigRow *combineRows(ScreenConfigElement * e, int top, int bottom);
    int getTopRow(ScreenConfigElement * e);
    int getBottomRow(ScreenConfigElement *e);
};

extern ScreenConfig *screenConfigDialog;
#endif // SCREENCONFIG_H

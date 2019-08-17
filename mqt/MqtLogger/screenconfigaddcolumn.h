#ifndef SCREENCONFIGADDCOLUMN_H
#define SCREENCONFIGADDCOLUMN_H

#include <QDialog>
class ScreenConfig;

namespace Ui {
class ScreenConfigAddColumn;
}

class ScreenConfigAddColumn : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenConfigAddColumn(ScreenConfig *parent);
    ~ScreenConfigAddColumn();

    int topRow = 0;
    int bottomRow = 0;
    bool addLeft = false;

private slots:

    void on_addColumnLeftButton_clicked();

    void on_addColumnRightButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::ScreenConfigAddColumn *ui;
    ScreenConfig *sc;
};

#endif // SCREENCONFIGADDCOLUMN_H

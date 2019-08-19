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

private slots:

    void on_cancelButton_clicked();

    void on_OKButton_clicked();

private:
    Ui::ScreenConfigAddColumn *ui;
    ScreenConfig *sc;
};

#endif // SCREENCONFIGADDCOLUMN_H

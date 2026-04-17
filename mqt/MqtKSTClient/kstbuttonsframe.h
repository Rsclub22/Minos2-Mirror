#ifndef KSTBUTTONSFRAME_H
#define KSTBUTTONSFRAME_H

#include "minospanel.h"
#include <QFrame>

namespace Ui {
class KSTButtonsFrame;
}

class KSTButtonsFrame : public MinosPanel
{
    Q_OBJECT

public:
    explicit KSTButtonsFrame(QWidget *parent = nullptr);
    ~KSTButtonsFrame();

    void on_FontChanged();

    void checkAwayButton();
    void setConnected(bool connected);
private slots:
    void on_closeButton_clicked();
    void on_connectButton_clicked();
    void on_configureButton_clicked();

    void on_layoutButton_clicked();

    void on_awayButton_clicked();

    void on_logsButton_clicked();

    void on_clearButton_clicked();

private:
    Ui::KSTButtonsFrame *ui;
};

#endif // KSTBUTTONSFRAME_H

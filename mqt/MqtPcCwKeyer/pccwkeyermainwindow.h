#ifndef PCCWKEYERMAINWINDOW_H
#define PCCWKEYERMAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include "pccwkeyer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class pcCwKeyerMainWindow;
}
QT_END_NAMESPACE

class pcCwKeyerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    pcCwKeyerMainWindow(QWidget *parent = nullptr);
    ~pcCwKeyerMainWindow();

private:
    Ui::pcCwKeyerMainWindow *ui;


    PcCwKeyer *cwKeyer = nullptr;
    QString pendingBuffer;

    QString comport;
    int wpm = 15;

    int farnsworth = -1;        // disable farnsworth

    bool sideToneOn = false;

    bool dtrRtsSelected = true; // true = dtr, false = rts




    void fillPortsInfo();

    void setWpmSpinnerRange(int minValue, int maxValue);
    void setWpmSpinnnerStep(int step);

    void setWpmValue(int value);
    int getWpmValue() const;
    void setConnections();
    void openCwKeyer();

    void closeCwKeyer();
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:
    //void onTextEdited(const QString &text);
    void checkCWBuffer();
    void onComportSelected();
    void onDtrSelected();
    void onSidetoneChkBoxSelected();
    void onRtsSelected();
    void onWpmValueChanged(int value);
    void onTextInputFinished(const QString &text);
};
#endif // PCCWKEYERMAINWINDOW_H


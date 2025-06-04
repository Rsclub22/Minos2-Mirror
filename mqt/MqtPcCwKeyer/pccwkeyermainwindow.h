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


    PcCwKeyer *cwKeyer;
    QString pendingBuffer;

private slots:
    void onTextEdited(const QString &text);
    void checkCWBuffer();
};
#endif // PCCWKEYERMAINWINDOW_H


#ifndef RSMAINWINDOW_H
#define RSMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class RSMainWindow; }
QT_END_NAMESPACE

class RSMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    RSMainWindow(QWidget *parent = nullptr);
    ~RSMainWindow();

private:
    Ui::RSMainWindow *ui;
};
#endif // RSMAINWINDOW_H

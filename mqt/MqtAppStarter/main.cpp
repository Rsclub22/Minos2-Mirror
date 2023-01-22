#include <QApplication>
#include <QMessageBox>

#include "AppStartup.h"
#include "SecondInstall.h"
#include "AsMainWindow.h"
#include "singleapplication.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    SingleApplication a( SecondInstall::getSingleAppStarterName(), argc, argv);

    if (a.isRunning())
    {
        QMessageBox msgBox;
        QString mess = QString(SecondInstall::getSingleAppStarterDescription() + " is already running! PID is %1").arg(a.getPid());
        msgBox.setText(mess);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.addButton("Close", QMessageBox::RejectRole);
        msgBox.exec();

        return -2;
    }

    appStartup("MinosQtAppStarter");

    MainWindow w;

    setAppFont();

    w.show();

    return a.exec();
}

#include "minos_pch.h"
#include "servermain.h"
#include "AppStartup.h"
#include "SecondInstall.h"

#include "singleapplication.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    SingleApplication a( SecondInstall::getSingleAppRouterName(), argc, argv);

    if (!a.isRunning())
    {
        appStartup("MinosQtServer");

        ServerMain w;
        w.show();

        return a.exec();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(SecondInstall::getSingleAppRouterDescription() + " is already running!");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.addButton("Close", QMessageBox::RejectRole);
        msgBox.exec();

    }
    return -2;
}

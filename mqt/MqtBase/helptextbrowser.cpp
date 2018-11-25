#include "helptextbrowser.h"
#include <QHelpEngine>
#include <QDesktopServices>

HelpTextBrowser::HelpTextBrowser(QWidget* parent):
                        QTextBrowser(parent)
    //  A subclass of QTextBrowser that handles Qt help collection files.
{
    QObject::connect(this, SIGNAL(anchorClicked(QUrl)), this, SLOT(on_anchorClicked(QUrl)));
}


QVariant HelpTextBrowser::loadResource(int type, const QUrl &url)
    //    Action slot for the anchorClicked signal.

    //    If url scheme is qthelp or file, set url to the
    //    help text browser source otherwise open
    //    the url with QDesktopServices
    //    for external links etc.

{
    QByteArray data;

    if (helpEngine && (url.scheme() == "qthelp"))
    {

        data = helpEngine->fileData(url);
        return QVariant(data);
    }
    else
    {
        return QTextBrowser::loadResource(type, url);
    }
}

void HelpTextBrowser::on_anchorClicked(const QUrl &url)
    //    Override inherited loadResource method.

    //    If url scheme is qthelp set the file data to the
    //    help text browser otherwise pass arguments
    //    on to the inherited method

{
    if ((url.scheme() == "qthelp") || (url.scheme() == "file"))
        QTextBrowser::setSource(url);
    else
        {
        QTextBrowser::reload();         // Prevents blank display
        QDesktopServices::openUrl(url); // Open in default web browser
        }
 }

void HelpTextBrowser::setHelpEngine(QHelpEngine *engine)
{
    helpEngine = engine;

}

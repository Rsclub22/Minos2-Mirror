#include <QApplication>
#include <QSettings>

#include "AppStartup.h"
#include "minospanel.h"

MinosPanel::MinosPanel(QWidget *parent ) : QFrame(parent)
{
    connect(&appStart, &AppStart::fontChanged, this, &MinosPanel::onSetFont);
}

void MinosPanel::setPanelFont()
{
    panelFont = QApplication::font();
    int pfontSize = panelFont.pointSize();
    int newSize = (pfontSize * fontsize)/100.0;
    panelFont.setPointSize(newSize);
    setFont( panelFont );   // WHICH SHOULD PROPAGATE - but doesn't

    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach(QWidget *w, widgets)
    {
        w->setFont(panelFont);
        w->update();
    }
 }
void MinosPanel::onSetFont()
{
    setPanelFont();
}

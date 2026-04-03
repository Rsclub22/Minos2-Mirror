#include <QApplication>
#include <QSettings>

#include "AppStartup.h"
#include "minospanel.h"

MinosPanel::MinosPanel(QWidget *parent ) : QFrame(parent)
{
    connect(&appStart, &AppStart::fontChanged, this, &MinosPanel::onSetFont);
}
void MinosPanel::onSetFont(QFont qfont)
{
    QFont panelFont = qfont;
    int fontSize = panelFont.pointSize();
    panelFont.setPointSize((fontSize * fontsize)/100.0);
    setFont( panelFont );   // WHICH SHOULD PROPAGATE - but doesn't

    for ( auto const &widget: QApplication::allWidgets() )
    {
        widget->setFont(panelFont);
        widget->update();
    }
}

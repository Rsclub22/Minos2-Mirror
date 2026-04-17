#ifndef MINOSPANEL_H
#define MINOSPANEL_H

#include <QFrame>

class MinosPanel : public QFrame
{
    Q_OBJECT
public:
    explicit MinosPanel(QWidget *parent = nullptr);

    virtual void setPanelFont();

    int fontsize = 100;
    QFont panelFont;
private slots:
    void onSetFont();
};

#endif // MINOSPANEL_H

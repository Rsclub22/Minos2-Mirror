#ifndef MINOSCUSTOMTAB_H
#define MINOSCUSTOMTAB_H

#include <QWidget>

// Allows the change of a background colour on the canvas of a tab placed on a QTabwidget.
// The default is Grey, but can be set to any colour with the setBackground method.


class MinosCustomTab : public QWidget
{

public:
    MinosCustomTab(QWidget *parent = nullptr);

    void setBackgroundColour(QString colour_);

protected:
    void paintEvent(QPaintEvent *event) override;


private:

    QString colour = "#f0f0f0";

};



#endif // MINOSCUSTOMTAB_H

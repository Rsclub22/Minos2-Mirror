#ifndef MINOSPANEL_H
#define MINOSPANEL_H

#include <QFrame>

class MinosPanel : public QFrame
{
    Q_OBJECT
public:
    explicit MinosPanel(QWidget *parent = nullptr);

    int fontsize = 100;
private slots:
    void onSetFont(QFont);
};

#endif // MINOSPANEL_H

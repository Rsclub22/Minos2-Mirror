#ifndef MINOSQLABEL_H
#define MINOSQLABEL_H

#include <QLabel>

class MinosQLabel : public QLabel
{
     Q_OBJECT

public:
    MinosQLabel(QWidget* parent = nullptr);
    virtual ~MinosQLabel();

    virtual void mouseDoubleClickEvent(QMouseEvent *ev) override;

signals:

    void mouseDoubleClicked();

};

#endif // MINOSQLABEL_H

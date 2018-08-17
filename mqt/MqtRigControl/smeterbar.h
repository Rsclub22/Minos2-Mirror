#ifndef SMETERBAR_H
#define SMETERBAR_H


#include <QTime>
#include <QWidget>


class SmeterBar : public QWidget
{
    Q_OBJECT

public:
    explicit SmeterBar(QWidget *parent = nullptr);
    ~SmeterBar();

    void paintEvent(QPaintEvent *event);


    void reset();
    void setLevel(qreal _level);


private:

    qreal slevel;




    QColor levelColor;
    QColor peakColor;
    QColor limitColor;
    QColor maxColor;

};


#endif // SMETERBAR_H

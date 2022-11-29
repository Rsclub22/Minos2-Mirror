#ifndef DATAPAINTER_H
#define DATAPAINTER_H

#include <QWidget>
#include <QStringList>

class DataPainter : public QWidget
{
    Q_OBJECT
public:
    explicit DataPainter(QWidget *parent = nullptr);
    virtual ~DataPainter()
    {}

    void setText(QStringList pText)
    {
        text = pText;
    }

signals:

protected:
    void paintEvent(QPaintEvent *e);

private:
    QStringList text;
};

#endif // DATAPAINTER_H

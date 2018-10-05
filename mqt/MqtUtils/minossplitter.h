#ifndef MINOSSPLITTER_H
#define MINOSSPLITTER_H

#include "mqtUtils_pch.h"
#include <QSplitterHandle>

class MinosSplitterHandle : public QSplitterHandle
{
    Q_OBJECT
public:
    MinosSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
    virtual ~MinosSplitterHandle() override
    {}

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
    QLinearGradient gradient;
};
//============================================================================
class MinosSplitter : public QSplitter
{
    Q_OBJECT
private:
   MinosSplitterHandle *myHandle;
public:
   MinosSplitter(QWidget *parent = nullptr);
   virtual ~MinosSplitter() override
   {}

protected:
   virtual QSplitterHandle *createHandle() override;

};
//============================================================================

#endif // MINOSSPLITTER_H

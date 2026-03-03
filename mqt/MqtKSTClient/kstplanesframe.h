#ifndef KSTPLANESFRAME_H
#define KSTPLANESFRAME_H

#include <QFrame>

namespace Ui {
class KSTPlanesFrame;
}

class KSTPlanesFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTPlanesFrame(QWidget *parent = nullptr);
    ~KSTPlanesFrame();

private:
    Ui::KSTPlanesFrame *ui;
};

#endif // KSTPLANESFRAME_H

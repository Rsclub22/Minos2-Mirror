#ifndef KSTBUTTONSFRAME_H
#define KSTBUTTONSFRAME_H

#include <QFrame>

namespace Ui {
class KSTButtonsFrame;
}

class KSTButtonsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTButtonsFrame(QWidget *parent = nullptr);
    ~KSTButtonsFrame();

private:
    Ui::KSTButtonsFrame *ui;
};

#endif // KSTBUTTONSFRAME_H

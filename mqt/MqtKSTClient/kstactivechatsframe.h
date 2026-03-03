#ifndef KSTACTIVECHATSFRAME_H
#define KSTACTIVECHATSFRAME_H

#include <QFrame>

namespace Ui {
class KSTActiveChatsFrame;
}

class KSTActiveChatsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTActiveChatsFrame(QWidget *parent = nullptr);
    ~KSTActiveChatsFrame();

private:
    Ui::KSTActiveChatsFrame *ui;
};

#endif // KSTACTIVECHATSFRAME_H

#ifndef KSTSENDMEEPFRAME_H
#define KSTSENDMEEPFRAME_H

#include <QFrame>

namespace Ui {
class KSTSendMeepFrame;
}

class KSTSendMeepFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTSendMeepFrame(QWidget *parent = nullptr);
    ~KSTSendMeepFrame();

private:
    Ui::KSTSendMeepFrame *ui;
};

#endif // KSTSENDMEEPFRAME_H

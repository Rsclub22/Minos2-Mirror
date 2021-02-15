#ifndef N1MMBROADCASTCONFIG_H
#define N1MMBROADCASTCONFIG_H

#include <QFrame>

namespace Ui {
class N1MMBroadcastConfig;
}

class N1MMBroadcastConfig : public QFrame
{
    Q_OBJECT

public:
    explicit N1MMBroadcastConfig(QWidget *parent = nullptr);
    virtual ~N1MMBroadcastConfig() override;

    void initialise();
    void finalise();

private:
    Ui::N1MMBroadcastConfig *ui;
};

#endif // N1MMBROADCASTCONFIG_H

#ifndef N1MMBROADCASTCONFIG_H
#define N1MMBROADCASTCONFIG_H

#include <QDialog>

namespace Ui {
class N1MMBroadcastConfig;
}

class N1MMBroadcastConfig : public QDialog
{
    Q_OBJECT

public:
    explicit N1MMBroadcastConfig(QWidget *parent = nullptr);
    ~N1MMBroadcastConfig();

private:
    Ui::N1MMBroadcastConfig *ui;
};

#endif // N1MMBROADCASTCONFIG_H

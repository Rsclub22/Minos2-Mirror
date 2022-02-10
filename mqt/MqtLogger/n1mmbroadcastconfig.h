#ifndef N1MMBROADCASTCONFIG_H
#define N1MMBROADCASTCONFIG_H

#include <QFrame>
#include "ConfigurationOption.h"


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

    bool check();
    void cancel();
private:
    Ui::N1MMBroadcastConfig *ui;

    ConfigurationOption contactsSelect;
    ConfigurationOption extCSSelect;
    ConfigurationOption ADIFSelect;

};

#endif // N1MMBROADCASTCONFIG_H

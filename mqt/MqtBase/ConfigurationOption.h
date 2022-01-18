#ifndef CONFIGURATIONOPTION_H
#define CONFIGURATIONOPTION_H

class SettingsBundle;
class QCheckBox;

class ConfigurationOption
{
    SettingsBundle *set = nullptr;
    int opt = -1;
    QCheckBox *cb = nullptr;
    bool initial = false;
public:
    ConfigurationOption()
    {}
    ~ConfigurationOption(){}

    void initialise(SettingsBundle *set, int opt, QCheckBox *cb);
    bool finalise();
    bool value();
};

#endif // CONFIGURATIONOPTION_H

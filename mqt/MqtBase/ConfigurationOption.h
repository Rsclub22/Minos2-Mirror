#ifndef CONFIGURATIONOPTION_H
#define CONFIGURATIONOPTION_H

#include <QString>


class SettingsBundle;
class QCheckBox;
class QLineEdit;

class ConfigurationOption
{
    SettingsBundle *set = nullptr;
    int opt = -1;
    QCheckBox *cb = nullptr;
    bool bInitial = false;
    QString sInitial;

    QString fileName;
    QString section;
    QString key;
    QLineEdit *qle = nullptr;

public:
    ConfigurationOption()
    {}
    ~ConfigurationOption(){}

    void initialise(SettingsBundle *set, int opt, QCheckBox *cb);
    void initialise(QString fileName, QString section, QString key, QLineEdit *qle, QString def);
    bool finalise() const;
    bool value() const;
    QString sValue() const;
};

#endif // CONFIGURATIONOPTION_H

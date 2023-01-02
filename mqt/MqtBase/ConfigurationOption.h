#ifndef CONFIGURATIONOPTION_H
#define CONFIGURATIONOPTION_H

#include <QString>


class SettingsBundle;
class QCheckBox;
class QLineEdit;
class QSpinBox;

class ConfigurationOption
{
    SettingsBundle *set = nullptr;
    int opt = -1;
    QCheckBox *cb = nullptr;
    bool bInitial = false;
    QLineEdit *qle = nullptr;
    QString sInitial;

    QSpinBox *sb = nullptr;
    int iInitial = 0;

    QString fileName;
    QString section;
    QString key;

public:
    ConfigurationOption()
    {}
    ~ConfigurationOption(){}

    void initialise(SettingsBundle *set, int opt, QCheckBox *cb);
    void initialise(SettingsBundle *set, int opt, QSpinBox *sb);
    void initialise(QString fileName, QString section, QString key, QLineEdit *qle, QString def);
    bool finalise() const;
    bool value() const;
    QString sValue() const;
    int iValue() const;
};

#endif // CONFIGURATIONOPTION_H

#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>

#include "profiles.h"

#include "ConfigurationOption.h"


void ConfigurationOption::initialise(SettingsBundle *pset, int popt, QLineEdit *pcb, QString def)
{
    set = pset;
    opt = popt;
    qle = pcb;

    set->getStringProfile(opt, sInitial);
    if (sInitial.isEmpty())
    {
        qle->setText(def);
    }
    else
    {
        qle->setText(sInitial);
    }
}
void ConfigurationOption::initialise(SettingsBundle *pset, int popt, QCheckBox *pcb)
{
    set = pset;
    opt = popt;
    cb = pcb;

    set->getBoolProfile( opt, bInitial );

    cb->setChecked(bInitial);




}




void ConfigurationOption::initialise(SettingsBundle *pset, int popt, QSpinBox *psb)
{
    set = pset;
    opt = popt;
    sb = psb;

    set->getIntProfile( opt, iInitial );
    sb->setValue(iInitial);
}


void ConfigurationOption::initialise(QString pfileName, QString psection, QString pkey, QLineEdit *pqle, QString def)
{
    fileName = pfileName;
    section = psection;
    key = pkey;
    qle = pqle;

    QSettings config(fileName, QSettings::IniFormat);
    sInitial = config.value( section + "/" + key, def ).toString();
    qle->setText(sInitial);


}
bool ConfigurationOption::finalise() const
{
    if (set)
    {
        if (cb)
        {
            bool now = cb->isChecked();
            if (now != bInitial)
            {
                set->setBoolProfile( opt, now );
                set->flushProfile();
                return true;
            }
        }
        else if (sb)
        {
            int now = sb->value();
            if (now != iInitial)
            {
                set->setIntProfile( opt, now );
                set->flushProfile();
                return true;
            }
        }
        else if (qle)
        {
            QString now = qle->text();
            if (now != sInitial)
            {
                set->setStringProfile(opt, now);
                set->flushProfile();
                return true;
            }
        }
    } else if (!fileName.isEmpty())
    {
        QString now = qle->text().trimmed();
        if (now != sInitial)
        {
            QSettings config(fileName, QSettings::IniFormat);
            config.setValue( section + "/" + key, now );
            return true;
        }
    }
    return false;
}

bool ConfigurationOption::value() const
{
    return cb->isChecked();
}

QString ConfigurationOption::sValue() const
{
    return qle->text().trimmed();
}
int ConfigurationOption::iValue() const
{
    return sb->value();
}

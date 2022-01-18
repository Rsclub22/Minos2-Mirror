#include "base_pch.h"
#include <QCheckBox>
#include "profiles.h"

#include "ConfigurationOption.h"


void ConfigurationOption::initialise(SettingsBundle *pset, int popt, QCheckBox *pcb)
{
    set = pset;
    opt = popt;
    cb = pcb;

    set->getBoolProfile( opt, initial );
    cb->setChecked(initial);

}
bool ConfigurationOption::finalise()
{
    bool now = cb->isChecked();
    if (now != initial)
    {
        set->setBoolProfile( opt, now );
        set->flushProfile();
        return true;
    }
    return false;
}

bool ConfigurationOption::value()
{
    return cb->isChecked();
}

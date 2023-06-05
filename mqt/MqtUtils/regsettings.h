#ifndef REGSETTINGS_H
#define REGSETTINGS_H

class QSettings;
class RegSettings
{
    static QSettings *s;
public:
    RegSettings();
    ~RegSettings();
    QSettings &getSettings();
};

#endif // REGSETTINGS_H

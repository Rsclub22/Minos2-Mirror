#ifndef KEYERJSON_H
#define KEYERJSON_H

#include <QString>
class KeyerJson
{
public:
    bool pipEnable = false;
    bool autoRepeat = false;
    int autoRepeatDelay = 0;

    int CQLength[10] = {};

    KeyerJson();

    bool parseConfig(QString);
    QString makeConfig();
};

#endif // KEYERJSON_H

#ifndef KEYERJSON_H
#define KEYERJSON_H

#include <QString>
#include <QVector>
#include <QJsonDocument>

const int KEYERKEYS = 8;    // to match VOICEKEYER_MAX_BUTTONS
class KeyerKeyJson
{
public:
    int keyno = -1;
    bool autoRepeat = false;
    int autoRepeatDelay = 0;

    int CQLength = 0;
    QString CQName;
};

class KeyerJson
{
public:
    bool pipEnable = false;
    int recordSliderPosition = 0;
    int replaySliderPosition = 0;
    int passthroughSliderPosition = 0;

    KeyerKeyJson kjj[KEYERKEYS];

    KeyerJson();

    bool parseConfig(QString);
    QString makeConfig(QJsonDocument::JsonFormat format);

    bool read(QString fileName);
    bool write(QString fileName);
};

#endif // KEYERJSON_H

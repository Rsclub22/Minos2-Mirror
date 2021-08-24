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
    QString makeConfig(QJsonDocument::JsonFormat format, bool force);

    bool read(QString fileName);
    bool write(QString fileName);
private:
    bool getBool(QJsonObject pe, QString key, bool def);
    int getInt(QJsonObject pe, QString key, int def);
    QString getString(QJsonObject o, QString key, QString def);
};

#endif // KEYERJSON_H

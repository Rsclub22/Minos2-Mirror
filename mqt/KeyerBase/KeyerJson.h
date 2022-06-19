#ifndef KEYERJSON_H
#define KEYERJSON_H

#include <QString>
#include <QVector>
#include <QJsonDocument>
#include "CompressorParams.h"

const int KEYERKEYS = 8;    // to match VOICEKEYER_MAX_BUTTONS
const int COMPRESSION_LIMIT = 30;
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
    CompressorParams compression;

    KeyerKeyJson kjj[KEYERKEYS];

    KeyerJson();

    bool parseConfig(QString, bool incSliders);
    QString makeConfig(QJsonDocument::JsonFormat format, bool force, bool incSliders);

    bool read(QString fileName);
    bool write(QString fileName);

    void traceConfig();

private:
    bool getBool(QJsonObject pe, QString key, bool def);
    int getInt(QJsonObject pe, QString key, int def);
    int getDouble(QJsonObject o, QString key, double def);
    QString getString(QJsonObject o, QString key, QString def);
};

#endif // KEYERJSON_H

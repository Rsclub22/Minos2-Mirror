#include <QFile>
#include <QSharedPointer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "QtUtils.h"

#include "MTrace.h"
#include "KeyerJson.h"

KeyerJson::KeyerJson()
{

}

bool KeyerJson::getBool(QJsonObject o, QString key, bool def)
{
    QJsonValue pe = o.value(key);
    if (pe.isBool())
    {
        return pe.toBool();
    }
    return def;
}
int KeyerJson::getInt(QJsonObject o, QString key, int def)
{
    QJsonValue pe = o.value(key);
    if (pe.isDouble())
    {
        return pe.toDouble();
    }
    return def;
}
int KeyerJson::getDouble(QJsonObject o, QString key, double def)
{
    QJsonValue pe = o.value(key);
    if (pe.isDouble())
    {
        return pe.toDouble();
    }
    return def;
}
QString KeyerJson::getString(QJsonObject o, QString key, QString def)
{
    QJsonValue pe = o.value(key);
    if (pe.isString())
    {
        return pe.toString();
    }
    return def;
}
bool KeyerJson::parseConfig(QString conf, bool incSliders)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(conf.toUtf8(), &err);
    if (!err.error)
    {
        if (json.isObject())
        {
            QJsonObject sconf = json.object();
            pipEnable = getBool(sconf, "pipEnable", false);

            if (incSliders)
            {
                recordSliderPosition = getInt(sconf, "record", 0);
                replaySliderPosition = getInt(sconf, "replay", 0);
                passthroughSliderPosition = getInt(sconf, "pass", 0);
                compression.read(sconf);
            }
            else
            {
                recordSliderPosition = -1000;
                replaySliderPosition = -1000;
                passthroughSliderPosition = -1000;
                compression = CompressorParams();
            }
            QJsonValue keys = sconf.value("keys");
            if (keys.isArray())
            {
                QJsonArray ac = keys.toArray();
                for (auto const &cf: QASCONST(ac))
                {
                    QJsonObject co = cf.toObject();

                    int knum = co.value("keyNum").toInt();
                    if (knum >= 0 && knum < KEYERKEYS)
                    {
                        KeyerKeyJson &k = kjj[knum];

                        k.CQLength = getInt(co, "CQLength", 0);
                        k.CQName = getString(co, "CQName", "");
                        k.autoRepeat = getBool(co, "autoRepeat", false);
                        k.autoRepeatDelay = getInt(co, "autoRepeatDelay", 0);
                    }
                }
            }
            return true;
        }
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + conf);
        return false;
    }


    return false;
}
QString KeyerJson::makeConfig(QJsonDocument::JsonFormat format, bool force, bool incSliders)
{
    QJsonDocument json;
    QJsonObject sconf;

    static int forceCount = 0;

    if (format == QJsonDocument::Compact)
    {
        sconf.insert("forceCount", forceCount);
        if (force)
        {
            forceCount++;
        }
    }

    sconf.insert("pipEnable", pipEnable);

    if (incSliders)
    {
        sconf.insert("record", recordSliderPosition);
        sconf.insert("replay", replaySliderPosition);
        sconf.insert("pass", passthroughSliderPosition);
        compression.insert(sconf);
    }

    QJsonArray ja;
    for (int i = 0; i < KEYERKEYS; i++)
    {
        QJsonObject ko;
        ko.insert("keyNum", i);

        ko.insert("CQLength", kjj[i].CQLength);
        ko.insert("CQName", kjj[i].CQName);
        ko.insert("autoRepeat", kjj[i].autoRepeat);
        ko.insert("autoRepeatDelay", kjj[i].autoRepeatDelay);

        ja.append(ko);

    }
    sconf.insert("keys", ja);
    json.setObject(sconf);

    QByteArray s = json.toJson(format);

    return QString(s);
}

bool KeyerJson::read(QString fileName)
{
    bool ret = false;
    QFile jf(fileName);
    if (jf.open(QIODevice::ReadOnly))
    {
        QString s = jf.readAll();
        ret = parseConfig(s, true);
    }
    return ret;
}

bool KeyerJson::write(QString fileName)
{
    QFile jf(fileName);
    if (!jf.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        trace("Failed to open " + fileName );
        return false;
    }

    QString conf = makeConfig(QJsonDocument::Indented, false, true);
    jf.write(conf.toUtf8());

    jf.close();

    return true;

}

void KeyerJson::traceConfig()
{
    trace(QString("masterconfig %1 %2 %3").arg(recordSliderPosition).arg(replaySliderPosition).arg(passthroughSliderPosition));
}

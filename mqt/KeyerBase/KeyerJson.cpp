#include "mqtUtils_pch.h"

#include <QSharedPointer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include "KeyerJson.h"

KeyerJson::KeyerJson()
{

}

bool KeyerJson::parseConfig(QString conf)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(conf.toUtf8(), &err);
    if (!err.error)
    {
        if (json.isObject())
        {
            QJsonObject sconf = json.object();
            QJsonValue pe = sconf.value("pipEnable");
            if (pe.isBool())
            {
                pipEnable = pe.toBool();
            }
            QJsonValue keys = sconf.value("keys");
            if (keys.isArray())
            {
                QJsonArray ac = keys.toArray();
                for (auto const &cf: qAsConst(ac))
                {
                    QJsonObject co = cf.toObject();

                    int knum = co.value("keyNum").toInt();
                    if (knum >= 0 && knum < KEYERKEYS)
                    {
                        KeyerKeyJson &k = kjj[knum];

                        k.CQLength = co.value("CQLength").toInt();
                        k.CQName = co.value("CQName").toString();
                        k.autoRepeat = co.value("autoRepeat").toBool();
                        k.autoRepeatDelay = co.value("autoRepeatDelay").toInt();
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
QString KeyerJson::makeConfig(QJsonDocument::JsonFormat format)
{
    QJsonDocument json;
    QJsonObject sconf;

    sconf.insert("pipEnable", pipEnable);

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
        ret = parseConfig(s);
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

    QString conf = makeConfig(QJsonDocument::Indented);
    jf.write(conf.toUtf8());

    jf.close();

    return true;

}

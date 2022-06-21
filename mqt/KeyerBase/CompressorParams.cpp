#include "CompressorParams.h"


CompressorParams::CompressorParams()
{
    qRegisterMetaType< CompressorParams > ( "CompressorParams" );

}
bool CompressorParams::getBool(QJsonObject o, QString key, bool def)
{
    QJsonValue pe = o.value(key);
    if (pe.isBool())
    {
        return pe.toBool();
    }
    return def;
}

double CompressorParams::getDouble(QJsonObject o, QString key, double def)
{
    QJsonValue pe = o.value(key);
    if (pe.isDouble())
    {
        return pe.toDouble();
    }
    return def;
}

void CompressorParams::read(QJsonObject sconf)
{
    CompressorParams def;
    // get params from sconf
    window = getDouble(sconf, "window", def.window);
    window = std::max(1.0, window);
     threshold = getDouble(sconf, "threshold", def.threshold);
     ratio = getDouble(sconf, "ratio", def.ratio);
     attack = getDouble(sconf, "attack", def.attack);
     attack = std::max(1.0, attack);
     release = getDouble(sconf, "release", def.release);
     release = std::max(1.0, release);
     makeUpGain = getDouble(sconf, "makeUpGain", def.makeUpGain);

     doCompression = getBool(sconf, "doCompresson", true);
     doFilter = getBool(sconf, "doFilter", true);


}

void CompressorParams::insert(QJsonObject &sconf)
{
    // insert params into sconf
    sconf.insert("window", window);
    sconf.insert("threshold", threshold);
    sconf.insert("ratio", ratio);
    sconf.insert("attack", attack);
    sconf.insert("release", release);
    sconf.insert("makeUpGain", makeUpGain);

    sconf.insert("doCompression", doCompression);
    sconf.insert("doFilter", doFilter);

}

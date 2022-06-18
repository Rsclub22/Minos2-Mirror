#ifndef COMPRESSORPARAMS_H
#define COMPRESSORPARAMS_H

#include <QSettings>
#include <QJsonObject>

class CompressorParams
{
public:
    CompressorParams();

    void read(QJsonObject sconf);
    void insert(QJsonObject &sconf);

    // set default params as "dry"
    double window = 1.0;       // milliseconds
    double threshold = 0.0;
    double ratio = 0.0001;
    double attack = 1.0;
    double release = 1.0;
    double makeUpGain = 0;  // db
private:
    double getDouble(QJsonObject o, QString key, double def);
};

#endif // COMPRESSORPARAMS_H

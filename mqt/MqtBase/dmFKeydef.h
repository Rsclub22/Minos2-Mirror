#ifndef DMFKEYDEF_H
#define DMFKEYDEF_H
#include <QString>

// vector of (map(key current set name) of vector of pairs (fkey name, fkey message) )
// added message repeat enable and repeat interval duration
// QVector is 24 entries, F1-F12, F1-F12
struct KeyVal {
            QString fk;
            QString ktop;
            QString kval;
            int rigVoiceMemNum = 0;
            bool rptEnable = false;
            int rptDur = 0;
};
typedef QVector<KeyVal >KeySet;
typedef QMap<QString, KeySet > Keys;

#endif // DMFKEYDEF_H

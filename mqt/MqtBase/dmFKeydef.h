#ifndef DMFKEYDEF_H
#define DMFKEYDEF_H
#include <QString>

// vector of (map(key current set name) of vector of pairs (fkey name, fkey message) )
// QVector is 24 entries, F1-F12, F1-F12
struct KeyVal {QString fk; QString ktop; QString kval;};
typedef QVector<KeyVal >KeySet;
typedef QMap<QString, KeySet > Keys;

#endif // DMFKEYDEF_H

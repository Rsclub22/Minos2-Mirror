#ifndef NETUTILS_H
#define NETUTILS_H

#include <QHostinfo>

bool isHostLocal(const QString &host);
bool processNetAddress(QLineEdit* networkAddBox, QString& netAddress, bool& radioValueChanged);

#endif // NETUTILS_H

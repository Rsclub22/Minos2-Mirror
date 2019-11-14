#include "minosNetUtils.h"

#include <QLineEdit>

bool isHostLocal(const QString &host)
{
    QHostInfo connaddr = QHostInfo::fromName( host );
    if ( connaddr.addresses().contains(QHostInfo::fromName( "127.0.0.1" ).addresses().first() ))
    {
       return true;             // connection address is 127.0.0.1
    }
    return false;
}



bool processNetAddress(QLineEdit* networkAddBox, QString& netAddress, bool& radioValueChanged)
{
    radioValueChanged = false;

    if (networkAddBox->text() != netAddress)
    {

        radioValueChanged = true;

        if (isHostLocal(networkAddBox->text().trimmed()))
        {
                return true;
        }
        else
        {
            QHostAddress address(networkAddBox->text());
            if (QAbstractSocket::IPv4Protocol == address.protocol())
            {
                return true;

            }
        }

    }

    return false;
}

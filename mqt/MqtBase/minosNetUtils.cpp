#include <QLineEdit>
#include "QtUtils.h"
#include "minosNetUtils.h"


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

        QHostInfo connaddr = QHostInfo::fromName( networkAddBox->text().trimmed() );

        if ( connaddr.addresses().contains(QHostInfo::fromName( "127.0.0.1" ).addresses().first() ))
        {
                return true;
        }
        else
        {
            QList<QHostAddress>addresses = connaddr.addresses();
            for (const auto &addr: QASCONST(addresses))
            {
                if (QAbstractSocket::IPv4Protocol == addr.protocol())
                {
                    return true;

                }
            }
        }

    }

    return false;
}

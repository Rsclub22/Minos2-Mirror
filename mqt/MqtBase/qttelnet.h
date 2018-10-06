/****************************************************************************
**
** This file is part of a Qt Solutions component.
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QTTELNET_H
#define QTTELNET_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QRegExp>
#include <QtNetwork/QTcpSocket>

class QtTelnetPrivate;

#if defined(Q_WS_WIN)
#  if !defined(QT_QTTELNET_EXPORT) && !defined(QT_QTTELNET_IMPORT)
#    define QT_QTTELNET_EXPORT
#  elif defined(QT_QTTELNET_IMPORT)
#    if defined(QT_QTTELNET_EXPORT)
#      undef QT_QTTELNET_EXPORT
#    endif
#    define QT_QTTELNET_EXPORT __declspec(dllimport)
#  elif defined(QT_QTTELNET_EXPORT)
#    undef QT_QTTELNET_EXPORT
#    define QT_QTTELNET_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTTELNET_EXPORT
#endif


namespace Common // RFC854
{
    // Commands
    const uchar CEOF  = 236;
    const uchar SUSP  = 237;
    const uchar ABORT = 238;
    const uchar SE    = 240;
    const uchar NOP   = 241;
    const uchar DM    = 242;
    const uchar BRK   = 243;
    const uchar IP    = 244;
    const uchar AO    = 245;
    const uchar AYT   = 246;
    const uchar EC    = 247;
    const uchar EL    = 248;
    const uchar GA    = 249;
    const uchar SB    = 250;
    const uchar WILL  = 251;
    const uchar WONT  = 252;
    const uchar DO    = 253;
    const uchar DONT  = 254;
    const uchar IAC   = 255;

    // Types
    const char IS    = 0;
    const char SEND  = 1;

    const char Authentication = 37; // RFC1416,
                                    // implemented to always return NULL
    const char SuppressGoAhead = 3; // RFC858
    const char Echo = 1; // RFC857, not implemented (returns WONT/DONT)
    const char LineMode = 34; // RFC1184, implemented
    const uchar LineModeEOF = 236, // RFC1184, not implemented
                LineModeSUSP = 237,
                LineModeABORT = 238;
    const char Status = 5; // RFC859, should be implemented!
    const char Logout = 18; // RFC727, implemented
    const char TerminalType = 24; // RFC1091,
                                  // implemented to always return UNKNOWN
    const char NAWS = 31; // RFC1073, implemented
    const char TerminalSpeed = 32; // RFC1079, not implemented
    const char FlowControl = 33; // RFC1372, should be implemented?
    const char XDisplayLocation = 35; // RFC1096, not implemented
    const char EnvironmentOld = 36; // RFC1408, should not be implemented!
    const char Environment = 39; // RFC1572, should be implemented
    const char Encrypt = 38; // RFC2946, not implemented

}


namespace Auth // RFC1416
{
    enum Auth
    {
        REPLY = 2,
        NAME
    };
    enum Types
    {
        AUTHNULL, // Can't have enum values named NULL :/
        KERBEROS_V4,
        KERBEROS_V5,
        SPX,
        SRA = 6,
        LOKI = 10
    };
    enum Modifiers
    {
        AUTH_WHO_MASK = 1,
        AUTH_CLIENT_TO_SERVER = 0,
        AUTH_SERVER_TO_CLIENT = 1,
        AUTH_HOW_MASK = 2,
        AUTH_HOW_ONE_WAY = 0,
        AUTH_HOW_MUTUAL = 2
    };
    enum SRA
    {
        SRA_KEY = 0,
        SRA_USER = 1,
        SRA_CONTINUE = 2,
        SRA_PASSWORD = 3,
        SRA_ACCEPT = 4,
        SRA_REJECT = 5
    };

}

namespace LineMode // RFC1184
{
    const char Mode = 1;
    const char ForwardMask = 2;
    const char SLC = 3;
    enum Modes
    {
        EDIT = 1,
        TRAPSIG = 2,
        MODE_ACK = 4,
        SOFT_TAB = 8,
        LIT_ECHO = 16
    };
    enum SLCs
    {
        SLC_SYNCH = 1,
        SLC_BRK = 2,
        SLC_IP = 3,
        SLC_AO =  4,
        SLC_AYT = 5,
        SLC_EOR = 6,
        SLC_ABORT = 7,
        SLC_EOF = 8,
        SLC_SUSP = 9,
        SLC_EC = 10,
        SLC_EL = 11,
        SLC_EW = 12,
        SLC_RP = 13,
        SLC_LNEXT = 14,
        SLC_XON = 15,
        SLC_XOFF = 16,
        SLC_FORW1 = 17,
        SLC_FORW2 = 18,
        SLC_MCL = 19,
        SLC_MCR = 20,
        SLC_MCWL = 21,
        SLC_MCWR = 22,
        SLC_MCBOL = 23,
        SLC_MCEOL = 24,
        SLC_INSRT = 25,
        SLC_OVER = 26,
        SLC_ECR = 27,
        SLC_EWR = 28,
        SLC_EBOL = 29,
        SLC_EEOL = 30,
        SLC_DEFAULT = 3,
        SLC_VALUE = 2,
        SLC_CANTCHANGE = 1,
        SLC_NOSUPPORT = 0,
        SLC_LEVELBITS = 3,
        SLC_ACK = 128,
        SLC_FLUSHIN = 64,
        SLC_FLUSHOUT = 32
    };
}

class QT_QTTELNET_EXPORT QtTelnet : public QObject
{
    Q_OBJECT
    friend class QtTelnetPrivate;
public:
    QtTelnet(QObject *parent = 0);
    ~QtTelnet();

    enum Control { GoAhead, InterruptProcess, AreYouThere, AbortOutput,
                   EraseCharacter, EraseLine, Break, EndOfFile, Suspend,
                   Abort };

    void connectToHost(const QString &host, quint16 port = 23);

    void login(const QString &user, const QString &pass);

    void setWindowSize(const QSize &size);
    void setWindowSize(int width, int height); // In number of characters
    QSize windowSize() const;
    bool isValidWindowSize() const;

    void setSocket(QTcpSocket *socket);
    QTcpSocket *socket() const;

    void setPromptPattern(const QRegExp &pattern);
    void setPromptString(const QString &pattern)
    { setPromptPattern(QRegExp(QRegExp::escape(pattern))); }
public Q_SLOTS:
    void close();
    void logout();
    void sendControl(Control ctrl);
    void sendData(const QString &data);
    void sendSync();
    void sConnected();

Q_SIGNALS:
    void loginRequired();
    void loginFailed();
    void loggedIn();
    void loggedOut();
    void connectionError(QAbstractSocket::SocketError error);
    void message(const QString &data);
    void socketConnected();

public:
    void setLoginPattern(const QRegExp &pattern);
    void setLoginString(const QString &pattern)
    { setLoginPattern(QRegExp(QRegExp::escape(pattern))); }
    void setPasswordPattern(const QRegExp &pattern);
    void setPasswordString(const QString &pattern)
    { setPasswordPattern(QRegExp(QRegExp::escape(pattern))); }

private:
    QtTelnetPrivate *d;
};
#endif

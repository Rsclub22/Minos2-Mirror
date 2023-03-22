//#include "MTrace.h"

#include "rxbuffer.h"

bool RXChar::getDirty() const
{
    return dirty;
}

void RXChar::setDirty(bool newDirty)
{
    dirty = newDirty;
}

int RXChar::getMarkFreq() const
{
    return markFreq;
}

void RXChar::setMarkFreq(int newMFreq)
{
    markFreq = newMFreq;
}

bool RXChar::getRST() const
{
    return RST;
}

void RXChar::setRST(bool newRST)
{
    RST = newRST;
}

bool RXChar::getSerial() const
{
    return serial;
}

void RXChar::setSerial(bool newSerial)
{
    serial = newSerial;
}

bool RXChar::getMyCall() const
{
    return myCall;
}

void RXChar::setMyCall(bool newMyCall)
{
    myCall = newMyCall;
}

bool RXChar::getWorkedCall() const
{
    return workedCall;
}

void RXChar::setWorkedCall(bool newWorkedCall)
{
    workedCall = newWorkedCall;
}

bool RXChar::getUnworkedCall() const
{
    return unworkedCall;
}

void RXChar::setUnworkedCall(bool newUnworkedCall)
{
    unworkedCall = newUnworkedCall;
}

RXChar::RXChar()
{

}

RXChar::RXChar(QChar c, bool nl, int dc, int carr)
{
    ch = c;
    newLine = nl;
    deleteCount = dc;
    valid = true;
    dirty = true;
    markFreq = carr;
}

bool RXChar::isType(const RXChar &rc) const
{
    if (ch == ' ' || rc.getCh() == ' ')
    {
        return false;
    }
    if (
        rc.RST == RST
        && rc.serial == serial
        && rc.myCall == myCall
        && rc.workedCall == workedCall
        && rc.unworkedCall == unworkedCall
    )
    {
        return true;
    }
    return false;
}

QChar RXChar::getCh() const
{
    return ch;
}

void RXChar::setCh(const QChar &newCh)
{
    ch = newCh;
}

void RXChar::setNewLine(bool newNewLine)
{
    newLine = newNewLine;
}

bool RXChar::getNewLine() const
{
    return newLine;
}

int RXChar::getDeleteCharacters() const
{
    return deleteCount;
}

bool RXChar::isValid() const
{
    return valid;
}
//=================================================
RxLine::RxLine()
{

}

bool RxLine::getDirty() const
{
    return dirty;
}

void RxLine::setDirty(bool newDirty)
{
    dirty = newDirty;
    if (!dirty)
    {
        for(auto &c:rxLine)
        {
            c.setDirty(false);
        }
    }
}

void RxLine::addChar(RXChar &c)
{
    rxLine.push_back(c);
    curCol++;
    setDirty(true);
}

void RxLine::reset()
{
    rxLine.clear();
    setDirty(true);
    curCol = 0;
}

int RxLine::charCount() const
{
    return rxLine.size();
}

void RxLine::clearFlags()
{
    for(auto &c:rxLine)
    {
        c.setRST(false);
        c.setSerial(false);
        c.setMyCall(false);
        c.setWorkedCall(false);
        c.setUnworkedCall(false);
    }
}

RXChar *RxLine::getCharRef(int col)
{
    if (col < rxLine.size())
    {
        return &rxLine[col];
    }
    return nullptr;
}

RXChar RxLine::getLastChar() const
{
    if (curCol > 0)
    {
        return rxLine[curCol - 1];
    }
    return RXChar();
}

RXChar RxLine::getCharAt(int col) const
{
    if (col < rxLine.size())
    {
        return rxLine[col];
    }
    return RXChar();
}

int RxLine::deleteChars(int n, RXChar &lastDeleted)
{
    while (curCol > 0 && n > 0)
    {
        curCol--;
        lastDeleted = rxLine[curCol];
        rxLine.remove(curCol);
        n--;
        //trace(QString("Remove char <%1> newline == %2").arg(lastDeleted.getCh()).arg(lastDeleted.getNewLine()));
        if (lastDeleted.getNewLine())
        {
            break;
        }
    }
    return n;
}

QString RxLine::toString()
{
    QString res;
    for (int i = 0; i < charCount(); i++)
    {
        res.append(rxLine[i].getCh());
    }
    return res;
}
//=================================================
int RxBuffer::getCurLine() const
{
    return curLine;
}

RxBuffer::RxBuffer()
{
    buff.resize(buffSize);
}

int RxBuffer::getLines() const
{
    return buff.size();
}

int RxBuffer::getCols(int line) const
{
    return buff[line].charCount();
}

void RxBuffer::addChar(RXChar &c)
{
    int d = c.getDeleteCharacters();
    deleteChars(d);

    if (c.getCh() == '\n' || c.getCh() == '\r')
    {
        //trace("Add newline character");
        c = RXChar(' ', true, 0, 0);
    }
    if (c.getNewLine())
    {
        //trace(QString("Implement newline from line %1").arg(curLine));

        int nextLine = curLine + 2;
        if (nextLine >= buffSize)
        {
            nextLine = 0;
        }
        if (buff[nextLine].charCount() > 0)
        {
            QString line = buff[nextLine].toString();
            emit newBackLine(line);
        }

        curLine++;
        if (curLine >= buffSize)
        {
            curLine = 0;
        }

        buff[curLine].reset();
        //trace(QString("clear line %1").arg(curLine));
        if (curLine + 1 < buffSize)
        {
            buff[curLine + 1].reset();

            RXChar ulc = RXChar('_', false, 0, 0);

            for (int i = 0; i < 40; i++)
            {
                buff[curLine + 1].addChar(ulc);
            }
            //trace(QString("clear line %1").arg(curLine + 1));
        }
    }

    buff[curLine].addChar(c);
    //trace(QString("Add character <%1> %2 on line %3").arg(c.getCh()).arg(int(c.getCh().toLatin1())).arg(curLine));

    emit newCharacter();
}

void RxBuffer::reset()
{
    curLine = 0;
    for(auto &l:buff)
    {
        l.reset();
    }
}

RXChar *RxBuffer::getCharRef(int line, int col)
{
    if (line < buffSize && col < buff[line].charCount())
    {
        return buff[line].getCharRef(col);
    }
    return nullptr;
}
RxLine *RxBuffer::getRxLine(int l)
{
    return &buff[l];
}

RXChar RxBuffer::getCharAt(int line, int col) const
{
    if (line < buffSize && col < buff[line].charCount())
    {
        return buff[line].getCharAt(col);
    }
    return RXChar();
}
int RxBuffer::charCount() const
{
    int n = 0;
    for (const auto &l:buff)
    {
        n += l.charCount();
    }
    return n;
}
void RxBuffer::deleteChars(int n)
{
    if (n > 0)
    {
        int l = charCount();
        if (n > l)
        {
            n = l;
        }
        if (n > 0)
        {
            //trace(QString("delete %1 characters curLine is %2").arg(n).arg(curLine));
            while (n > 0)
            {
                RXChar lastDeleted;  // but if there is more than one char...
                n = buff[curLine].deleteChars(n, lastDeleted);
                int l = charCount();
                if (n > l)
                {
                    n = l;
                }

                if (lastDeleted.getNewLine())
                {
                    buff[curLine].reset();
                    curLine--;
                    if (curLine < 0)
                    {
                        curLine = buffSize - 1;
                    }
                    //trace(QString("back one line to %1").arg(curLine));
                }
            }
        }
    }
}

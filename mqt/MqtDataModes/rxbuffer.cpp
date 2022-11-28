#include "MTrace.h"

#include "rxbuffer.h"

RXChar::RXChar()
{

}

RXChar::RXChar(QChar c, bool nl, int dc)
{
    ch = c;
    newLine = nl;
    deleteCount = dc;
    valid = true;
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
    return buff[line].size();
}

void RxBuffer::addChar(RXChar &c)
{
    int d = c.getDeleteCharacters();
    deleteChars(d);

    if (c.getCh() == '\n' || c.getCh() == '\r')
    {
        trace("Add newline character");
        c = RXChar(' ', true, 0);
    }
    if (c.getNewLine())
    {
        trace(QString("Implement newline charcount is %1").arg(charCount));
        curLine++;
        curCol = 0;
        if (curLine >= buffSize)
        {
            curLine = 0;
        }
        charCount -= buff[curLine].size();
        buff[curLine].clear();
        trace(QString("clear line %1 charcount is %2").arg(curLine).arg(charCount));
        if (curLine + 1 < buffSize)
        {
            charCount -= buff[curLine + 1].size();
            buff[curLine + 1].clear();
            trace(QString("clear line %1 charcount is %2").arg(curLine + 1).arg(charCount));
        }
    }

    buff[curLine].push_back(c);
    curCol++;
    charCount++;
    trace(QString("Add character <%1> %2 charCount = %3").arg(c.getCh()).arg(int(c.getCh().toLatin1())).arg(charCount));

    emit newCharacter();
}

void RxBuffer::reset()
{
    curLine = 0;
    curCol = 0;
    for(auto l:qAsConst(buff))
    {
        l.clear();
    }
    charCount = 0;
}

RXChar RxBuffer::getCharAt(int line, int col) const
{
    if (line < buffSize && col < buff[line].size())
    {
        return buff[line][col];
    }
    return RXChar();
}
void RxBuffer::deleteChars(int n)
{
    if (n > 0)
    {
        trace(QString("delete %1 characters").arg(n));
        for (int i = 0; i < n; i++)
        {
            if (n - i > charCount)
            {
                trace("not enough characters to remove");
                break;
            }
            if (curCol > 0)
            {
                curCol--;
                RXChar dch = buff[curLine][curCol];
                buff[curLine].remove(curCol);
                charCount--;
                trace(QString("Removed last char on line charcount = %1").arg(charCount));
                if (dch.getNewLine())
                {
                    trace("go back a line as del char has newline");
                    curLine--;
                    if (curLine < 0)
                    {
                        curLine = buffSize - 1;
                    }
                }
            }
            else
            {
                trace("go back a line");
                buff[curLine].clear();
                curLine--;
                if (curLine < 0)
                {
                    curLine = buffSize - 1;
                }
                curCol = buff[curLine].size();
                deleteChars(n - i);
            }
        }
    }
}

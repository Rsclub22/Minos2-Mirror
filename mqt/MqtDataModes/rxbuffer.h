#ifndef RXBUFFER_H
#define RXBUFFER_H

#include <QObject>
#include <QVector>
//=================================================

class RXChar
{
    QChar ch = ' ';
    bool newLine = false;
    bool valid = false;
    int deleteCount = 0;
    bool dirty = false;
    int carrier = 0;
public:
    RXChar();
    RXChar(QChar c, bool nl, int dc, int carr);

    QChar getCh() const;
    void setCh(const QChar &newCh);
    bool getNewLine() const;
    int getDeleteCharacters() const;
    bool isValid() const;
    void setNewLine(bool newNewLine = true);
    bool getDirty() const;
    void setDirty(bool newDirty);
    int getCarrier() const;
    void setCarrier(int newCarrier);
};
//=================================================

class RxLine
{
private:
    QVector<RXChar> rxLine;
    int curCol = 0;
    bool dirty = false;
public:
    RxLine();
    bool getDirty() const;
    void setDirty(bool newDirty);
    void  addChar(RXChar &c);
    void reset();
    int size() const;

    RXChar getLastChar() const;
    RXChar getCharAt(int col) const;
    int deleteChars(int n, RXChar &lastDeleted); // return chars still to delete
};
//=================================================

class RxBuffer: public QObject
{
    Q_OBJECT
private:
    QVector< RxLine > backLines;

    QVector< RxLine > buff;
    int curLine = 0;

    int buffSize = 15;  // lines
public:
    static RxBuffer *getRxBuffer()
    {
        static RxBuffer buff;
        return &buff;
    }
    RxBuffer();
    void  addChar(RXChar &c);
    void reset();

    RXChar getCharAt(int line, int col) const;
    void deleteChars(int n);

    RxLine *getRxLine(int);

    int getLines() const;

    int getCols(int line) const;

    int charCount() const;

    const RxLine *getBackLine(int) const;
    int getBackLines() const;


signals:
    void newCharacter();
};

#endif // RXBUFFER_H

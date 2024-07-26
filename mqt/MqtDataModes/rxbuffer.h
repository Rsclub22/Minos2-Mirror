#ifndef RXBUFFER_H
#define RXBUFFER_H

#include <QObject>
#include <QVector>
//=================================================

class RXChar
{
    QChar ch = ' ';
    bool valid = false;
    int deleteCount = 0;
    bool dirty = false;
    int markFreq = 0;

    bool RST = false;
    bool serial = false;
    bool myCall = false;
    bool workedCall = false;
    bool unworkedCall = false;
public:
    RXChar();
    RXChar(QChar c, int dc, int carr);

    bool isType(const RXChar &) const;

    QChar getCh() const;
    void setCh(const QChar &newCh);
    int getDeleteCharacters() const;
    bool isValid() const;
    bool getDirty() const;
    void setDirty(bool newDirty);
    int getMarkFreq() const;
    void setMarkFreq(int newMFreq);
    bool getRST() const;
    void setRST(bool newRST);
    bool getSerial() const;
    void setSerial(bool newSerial);
    bool getMyCall() const;
    void setMyCall(bool newMyCall);
    bool getWorkedCall() const;
    void setWorkedCall(bool newWorkedCall);
    bool getUnworkedCall() const;
    void setUnworkedCall(bool newUnworkedCall);
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
    int charCount() const;
    void clearFlags();

    RXChar *getCharRef(int col);
    RXChar getLastChar() const;
    RXChar getCharAt(int col) const;
    int deleteChars(int n); // return chars still to delete
    QString toString();
};
//=================================================

class RxBuffer: public QObject
{
    Q_OBJECT
private:
    QVector< RxLine > buff;
    int curLine = 0;

    int buffSize = 15;  // lines
public:
    RxBuffer();
    void  addChar(RXChar &c);
    void reset();

    RXChar *getCharRef(int line, int col);
    RXChar getCharAt(int line, int col) const;
    void deleteChars(int n);

    RxLine *getRxLine(int);

    int getLines() const;

    int getCols(int line) const;

    int charCount() const;

    int getCurLine() const;

signals:
    void newCharacter();
};

#endif // RXBUFFER_H

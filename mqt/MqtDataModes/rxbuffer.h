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

    bool RST = false;
    bool serial = false;
    bool myCall = false;
    bool workedCall = false;
    bool unworkedCall = false;
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
    int deleteChars(int n, RXChar &lastDeleted); // return chars still to delete
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
    void newBackLine(QString);
};

#endif // RXBUFFER_H

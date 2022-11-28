#ifndef RXBUFFER_H
#define RXBUFFER_H

#include <QObject>
#include <QVector>

class RXChar
{
    QChar ch = ' ';
    bool newLine = false;
    bool valid = false;
    int deleteCount = 0;
public:
    RXChar();
    RXChar(QChar c, bool nl, int dc);

    QChar getCh() const;
    void setCh(const QChar &newCh);
    bool getNewLine() const;
    int getDeleteCharacters() const;
    bool isValid() const;
    void setNewLine(bool newNewLine = true);
};

class RxBuffer: public QObject
{
    Q_OBJECT
private:
    QVector< QVector<RXChar> > buff;
    int curLine = 0;
    int curCol = 0;

    int buffSize = 15;  // lines
    int charCount = 0;
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

    int getLines() const;

    int getCols(int line) const;

signals:
    void newCharacter();
};

#endif // RXBUFFER_H

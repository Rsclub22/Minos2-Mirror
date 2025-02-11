#ifndef DMBUTTONFRAME_H
#define DMBUTTONFRAME_H

#include "AnalysePubSubNotify.h"
#include "frequency.h"
#include <QFrame>
#include <QMap>
#include <QVector>

class QPushButton;
class QFileSystemWatcher;
class LoggerContestLog;
class BaseContestLog;

namespace Ui {
class DMButtonFrame;
}

class DMButtonFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DMButtonFrame(QWidget *parent = nullptr);
    ~DMButtonFrame();

    void setContest(BaseContestLog *);

    QString parseFKeyMessage(QString mess);
    void parseFKeyFile(QString sfname, QString mode);
private slots:
    void fKey(BaseContestLog *c, int key, int carr);
    void sandPChanged(bool);
    void fButtonClicked();
    void on_stopButton_clicked();

    void on_editButton_clicked();

    void on_logitButton_clicked();

    void on_wipeButton_clicked();

    void on_chooseButton_clicked();

    void fkeyFileChanged();
    void DMMess(AnalysePubSubNotify an);
    void onModeChange(QString mode);
    void on_fkeysetCombo_textActivated(const QString &arg1);

private:
    Ui::DMButtonFrame *ui;
    LoggerContestLog *ct = nullptr;

    QString fkeyFileName;
    QFileSystemWatcher *qfsw = nullptr;
    QStringList nameList;
    QString currentName = "Default";

    QVector<QPushButton *> fButtons;

    // map(key contest type) of pairs (first map(key Fkey) of run message , second map(key fKey) of S&P message)
    //typedef QMap<QString, QPair<QMap<QString, QString>, QMap<QString, QString> > > Keys;

    // map(key "Digi" of (map(key current set name) of vector of pairs (fkey name, fkey mesage) ))
    typedef QMap<QString, QMap<QString, QVector<QPair<QString, QString> > > > Keys;
    Keys fkeys;

    QString dataSender;
    QString curMode;

    void showFButtons(bool s);
    QString getFKeysString() const;
    bool parseFKeyString(QString s, QString mode);
    bool parseFKeyArray(QJsonArray s, QString keyset, QString mode);
    bool isDataMode();
};

#endif // DMBUTTONFRAME_H

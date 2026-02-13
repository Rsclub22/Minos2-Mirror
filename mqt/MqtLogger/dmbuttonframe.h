#ifndef DMBUTTONFRAME_H
#define DMBUTTONFRAME_H

#include "AnalysePubSubNotify.h"
#include <QFrame>
#include <QMap>
#include <QVector>

#include "dmFKeydef.h"
#include "frequency.h"

class QPushButton;
class QFileSystemWatcher;
class LoggerContestLog;
class BaseContestLog;
class RigControlFrame;

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
    void parseFKeyFile(QString sfname);
    void setFreq(Frequency f);
signals:
    void sendFreqControl(Frequency f);
private slots:
    void fKey(BaseContestLog *c, int key, int carr);
    void sandPChanged(BaseContestLog *c, bool sandp);
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
    Frequency curFreq;
    QString fkeyFileName;
    QFileSystemWatcher *qfsw = nullptr;
    QStringList nameList;
    QString currentName = "Default";

    QVector<QPushButton *> fButtons;

    Keys fkeys;

    QString dataSender;
    QString curMode;

    void showFButtons(bool sandp);
    QString getFKeysString() const;
    bool parseFKeyString(QString s);
    bool parseFKeyArray(QJsonArray s, QString keyset);
    void rewriteFKeyFile();

    bool isDataMode();
};

#endif // DMBUTTONFRAME_H

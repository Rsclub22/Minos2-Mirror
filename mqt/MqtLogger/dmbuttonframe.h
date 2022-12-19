#ifndef DMBUTTONFRAME_H
#define DMBUTTONFRAME_H

#include <QFrame>
#include <QMap>
#include <QVector>

class QPushButton;
class QFileSystemWatcher;
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
    void fKey(int key);
    void sandPChanged(bool);
    void fButtonClicked();
    void on_stopButton_clicked();

    void on_editButton_clicked();

    void on_logitButton_clicked();

    void on_wipeButton_clicked();

    void on_chooseButton_clicked();

    void fkeyFileChanged();
    void DMSender(QString s);
private:
    Ui::DMButtonFrame *ui;
    BaseContestLog *ct = nullptr;

    QString fkeyFileName;
    QFileSystemWatcher *qfsw = nullptr;

    QVector<QPushButton *> fButtons;
    QMap<QString, QVector<QPair<QString, QString> > > fkeys;

    QString dataSender;

    void showFButtons(bool s);
};

#endif // DMBUTTONFRAME_H

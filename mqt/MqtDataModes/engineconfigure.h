#ifndef ENGINECONFIGURE_H
#define ENGINECONFIGURE_H

#include <QDialog>

namespace Ui {
class EngineConfigure;
}

class QLineEdit;
class DMMainWindow;

class EngineConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit EngineConfigure(DMMainWindow *parent = nullptr);
    ~EngineConfigure();

    static QString getEnginePath(QString engine);
    static void setEnginePath(QString engine, QString path);

    static QString getSpeed(QString mode);
    static void setSpeed(QString mode, QString speed);

    static bool getEngineEnabled(QString engine);
    static void setEngineEnabled(QString engine, bool s);

    static QString getEngineSound(QString engine, QString io);
    static void setEngineSound(QString engine, QString io, QString s);

    static int getEnginePort(QString engine);
    static void setEnginePort(QString engine, int port);
private slots:
    void on_mmttyBrowse1_clicked();
    void on_mmttyBrowse2_clicked();

    void on_twotoneBrowse1_clicked();
    void on_twotoneBrowse2_clicked();

    void on_fldigiBrowse_clicked();

    void on_grittyBrowse_clicked();

    void on_OKButton_clicked();

    void on_cancelButton_clicked();
    void on_senderCombo_activated(int index);

private:
    Ui::EngineConfigure *ui;
    DMMainWindow *mainWindow;
    QString geoStr;
    void doBrowse(QString key, QLineEdit *edit);
    bool checkEnginesAvailable();
    virtual void closeEvent(QCloseEvent *event) override;

};

#endif // ENGINECONFIGURE_H

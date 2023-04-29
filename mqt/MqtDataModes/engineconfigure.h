#ifndef ENGINECONFIGURE_H
#define ENGINECONFIGURE_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class EngineConfigure;
}

class QLineEdit;
class QCheckBox;

class DMMainWindow;

class EngineConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit EngineConfigure(DMMainWindow *parent = nullptr);
    ~EngineConfigure();


    static QString getEnginePath(QSettings &settings, QString engine);
    static QString getEnginePath(QString engine);
    static void setEnginePath(QSettings &settings, QString engine, QString path);

    static QString getSpeed(QSettings &settings,QString mode);
    static QString getSpeed(QString mode);
    static void setSpeed(QSettings &settings,QString mode, QString speed);

    static bool getEngineEnabled(QSettings &settings,QString engine);
    static bool getEngineEnabled(QString engine);
    static void setEngineEnabled(QSettings &settings,QString engine, bool s);

    static QString getEngineSound(QSettings &settings,QString engine, QString io);
    static QString getEngineSound(QString engine, QString io);
    static void setEngineSound(QSettings &settings,QString engine, QString io, QString s);

    static int getEnginePort(QSettings &settings,QString engine);
    static int getEnginePort(QString engine);
    static void setEnginePort(QSettings &settings,QString engine, int port);

    static QString getEnginePTT(QSettings &settings, QString engine);
    static QString getEnginePTT(QString engine);
    static void setEnginePTT(QSettings &settings,QString engine, QString port);

    static int getEnginePTTL(QSettings &settings, QString engine);
    static int getEnginePTTL(QString engine);
    static void setEnginePTT(QSettings &settings,QString engine, int l);
private slots:
    bool check();

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

    void checkEnginePath(QLineEdit *ele, QCheckBox *ecb);
    void checkEnginePort(QLineEdit *ele, QCheckBox *ecb);
    void setPortDefault(QSettings &settings, QString engine, QLineEdit *ple, int def);
};

#endif // ENGINECONFIGURE_H

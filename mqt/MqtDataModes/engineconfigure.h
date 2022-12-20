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

    static QString getAppPath(QString engine);

    static void setAppPath(QString engine, QString path);

    static QString getAppCurrent();

    static void setAppCurrent(QString engine);

private slots:
    void on_mmttyBrowse_clicked();

    void on_twotoneBrowse_clicked();

    void on_fldigiBrowse_clicked();

    void on_grittyBrowse_clicked();

    void on_OKButton_clicked();

    void on_cancelButton_clicked();
private:
    Ui::EngineConfigure *ui;
    DMMainWindow *mainWindow;
    void doBrowse(QString key, QLineEdit *edit);
};

#endif // ENGINECONFIGURE_H

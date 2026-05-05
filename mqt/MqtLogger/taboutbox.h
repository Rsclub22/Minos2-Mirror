#ifndef TABOUTBOX_H
#define TABOUTBOX_H

#include <QDialog>

namespace Ui {
class TAboutBox;
}

class TAboutBox : public QDialog
{
    Q_OBJECT

    static QString creditsText;
    static QString MinosText;

    Ui::TAboutBox *ui;
    explicit TAboutBox(QWidget *parent, bool onStartup );
    ~TAboutBox() override ;

    int exec() override;

    void doCloseEvent();
    void showAppConfig();

public:
    static bool ShowAboutBox(QWidget *Owner, bool onStartup );
    bool doStartup;


public slots:
    virtual void reject() override;

    virtual void accept() override;

private slots:
    void on_ExitButton_clicked();

    void on_OKButton_clicked();

    void on_LoggerOnlyButton_clicked();

    void on_AboutMemo_linkActivated(const QString &link);
    void on_AppsButton_clicked();
    void on_manageSets_clicked();
    void on_ageSpinner_valueChanged(int arg1);
    void on_ageCb_stateChanged(int arg1);
    void on_appSelectButton_clicked();
    void on_fontButton_clicked();
    void on_newHFButton_clicked();
    void on_newVHFButton_clicked();
private:
    void doNewContest(bool hf);
};

#endif // TABOUTBOX_H

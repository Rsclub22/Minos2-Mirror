#ifndef TABOUTBOX_H
#define TABOUTBOX_H

#include "base_pch.h"

namespace Ui {
class TAboutBox;
}

class TAboutBox : public QDialog
{
    Q_OBJECT

    static QString welcomeText;
    static QString creditsText;
    static QString MinosText;

    Ui::TAboutBox *ui;
    explicit TAboutBox(QWidget *parent, bool onStartup );
    ~TAboutBox() override ;

    int exec() override;

    void doCloseEvent();

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
};

#endif // TABOUTBOX_H

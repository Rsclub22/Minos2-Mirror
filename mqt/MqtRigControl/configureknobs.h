#ifndef CONFIGUREKNOBS_H
#define CONFIGUREKNOBS_H

#include <QDialog>

namespace Ui {
class ConfigureKnobs;
}

class ConfigureKnobs : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureKnobs(QWidget *parent = nullptr);
    ~ConfigureKnobs();

    static void start();
    static void stop();

private slots:
    void on_pushButton_clicked();

    void on_knobCombo_activated(int index);

private:
    Ui::ConfigureKnobs *ui;

    int curKnob = 0;

    void done(int r) override;   // override done function to validate data entry
    void closeEvent(QCloseEvent *event)override;
    void doCloseEvent();

};

#endif // CONFIGUREKNOBS_H

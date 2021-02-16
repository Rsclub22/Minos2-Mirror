#ifndef DISPLAYOPTIONS_H
#define DISPLAYOPTIONS_H

#include <QFrame>

namespace Ui {
class DisplayOptions;
}

class DisplayOptions : public QFrame
{
    Q_OBJECT

public:
    explicit DisplayOptions(QWidget *parent = nullptr);
    virtual ~DisplayOptions() override;

    void initialise();
    void finalise();

private slots:
    void on_FontChangeButton_clicked();

private:
    Ui::DisplayOptions *ui;

    bool so = false;
    bool autoFill = false;
    bool TabSandP = false;
    int cap = 0;
    int lcf;
    int qff;
    QFont f;
    QFont nf;

    int currLang = -1;
    bool doBounceOnExit = false;

    void doFontChange();

    void doLanguageChange();
};

#endif // DISPLAYOPTIONS_H

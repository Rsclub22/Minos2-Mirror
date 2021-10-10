#ifndef DISPLAYOPTIONS_H
#define DISPLAYOPTIONS_H

#include <QFrame>
#include "ProfileEnums.h"

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

    bool check();
    void cancel();
private slots:
    void on_FontChangeButton_clicked();

private:
    Ui::DisplayOptions *ui;

    bool so = false;
    bool reportReadabilityInitialise = false;
    bool reportStrengthAutoFill = false;
    bool TabSandP = false;
    int lcf;
    int qff;
    QFont f;
    QFont nf;

    int currLang = -1;
    bool doBounceOnExit = false;

    SHOWOPERATINGTIME sot = otNone;

    int lm;
    int ls;
    int cml;
    int cmt;
    int cmr;
    int cmb;

    bool sepIcons;

    void doFontChange();

    void doLanguageChange();
};

#endif // DISPLAYOPTIONS_H

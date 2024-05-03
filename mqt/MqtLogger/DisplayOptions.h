#ifndef DISPLAYOPTIONS_H
#define DISPLAYOPTIONS_H

#include <QFrame>
#include "ProfileEnums.h"
#include "ConfigurationOption.h"

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

    ConfigurationOption ShowOperators;
    ConfigurationOption ReadabilityInit;
    ConfigurationOption AutoFill;
    ConfigurationOption TabforSandP;
    ConfigurationOption SeparateIcons;
    ConfigurationOption ExpertMode;
    ConfigurationOption AlternateFKeys;
    ConfigurationOption ShowAuxHeaders;
    ConfigurationOption ShowSingleBandInCrib;
    ConfigurationOption ShowQSOMapGrid;
    ConfigurationOption ShowQSOMapLines;
    ConfigurationOption ShowQSOMapShowLoc;
    ConfigurationOption ShowQSOMapTLLoc;
    ConfigurationOption ShowQSOMapBRLoc;
    ConfigurationOption ShowQSOMapShowNav;
    ConfigurationOption MapShowCluster;
    ConfigurationOption MapClusterDistance;

    int lcf;
    int qff;
    QFont f;
    QFont nf;

    int currLang = -1;
    bool doBounceOnExit = false;

    SHOWOPERATINGTIME sot = otNone;
    LOCMAPCENTRE lmc = lmsMyLoc;

    int lm;
    int ls;
    int cml;
    int cmt;
    int cmr;
    int cmb;

    void doFontChange();

    void doLanguageChange();
};

#endif // DISPLAYOPTIONS_H

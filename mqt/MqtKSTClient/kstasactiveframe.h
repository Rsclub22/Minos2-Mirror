#ifndef KSTASACTIVEFRAME_H
#define KSTASACTIVEFRAME_H

#include <QFrame>

#include "airscoutlink.h"
#include "minospanel.h"

namespace Ui {
class KSTASActiveFrame;
}

class KSTASActiveFrame : public MinosPanel
{
    Q_OBJECT

public:
    explicit KSTASActiveFrame(QWidget *parent = nullptr);
    ~KSTASActiveFrame();

    void setASBands(QVector<const char *> ASBandStrings);
    bool getASActive() const;
    ASBand getASActiveBand() const;
    void setASActive(bool s);
    void do_asBandCombo_currentIndexChanged(int band);
    void on_FontChanged();
private slots:
    void on_ASActivecb_stateChanged(int state);
    void on_asBandCombo_currentIndexChanged(int band);

private:
    Ui::KSTASActiveFrame *ui;
};

#endif // KSTASACTIVEFRAME_H

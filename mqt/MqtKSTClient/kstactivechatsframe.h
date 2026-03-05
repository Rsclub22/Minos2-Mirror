#ifndef KSTACTIVECHATSFRAME_H
#define KSTACTIVECHATSFRAME_H

#include "airscoutlink.h"
#include <QFrame>

namespace Ui {
class KSTActiveChatsFrame;
}
class QCheckBox;
class QRadioButton;
class KSTActiveChatsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTActiveChatsFrame(QWidget *parent = nullptr);
    ~KSTActiveChatsFrame();

    void on_FontChanged();

    void setActive(int chat);
    void checkActive();
    void setLoginTexts(QStringList services);
    void setASBands(QVector<const char *> ASBandStrings);
    void resetVectors(QCheckBox *cb, int c, QStringList &s, QVector<int> &v, QVector<int> &a);
    bool getASActive() const;
    ASBand getASActiveBand() const;
private slots:
    void on_ASActivecb_stateChanged(int state);

    void activerb_clicked();
    void on_asBandCombo_currentIndexChanged(int band);
private:
    Ui::KSTActiveChatsFrame *ui;
};

#endif // KSTACTIVECHATSFRAME_H

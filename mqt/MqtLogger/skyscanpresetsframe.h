#ifndef SKYSCANPRESETSFRAME_H
#define SKYSCANPRESETSFRAME_H

#include <QFrame>
#include "minospanel.h"
#include "presetbutton.h"


namespace Ui {
class skyScanPresetsFrame;
}

class skyScanPresetsFrame : public MinosPanel
{
    Q_OBJECT

public:
    explicit skyScanPresetsFrame(QWidget *parent = nullptr);
    ~skyScanPresetsFrame();

    void setPresetList(QString skyScanPresetList);
    void setSkyCanVisible(bool visible);
signals:
    void recallSkyScanPreset(int buttonNumber);

private slots:
    void presetRead(int buttonNumber);
private:
    Ui::skyScanPresetsFrame *ui;

    void initPresetButtons();

    QList<QSharedPointer<PresetButton>> skyScanPresetButton;
    void traceMessage(QString msg);
};

#endif // SKYSCANPRESETSFRAME_H

#ifndef CONFIGELEMENTFRAME_H
#define CONFIGELEMENTFRAME_H

#include "base_pch.h"
#include "ConfigFile.h"

namespace Ui {
class ConfigElementFrame;
}

class ConfigElementFrame : public QFrame
{
    Q_OBJECT
static const char *deltext;
    QSharedPointer<RunConfigElement> configElement;
    bool newElement = false;

    bool inhibitIndexChange = false;

    QLinearGradient gradient;

protected:
    void paintEvent(QPaintEvent *event);
public:
    explicit ConfigElementFrame(bool nele);
    ~ConfigElementFrame();

    bool localOK;
    bool remoteOK;

    void checkEnabled();
    void setElement(QSharedPointer<RunConfigElement> configElement);
    void saveElement();
    void setNameFocus();
    void fixComboStyle();


private slots:
    void on_programBrowseButton_clicked();

    void on_homeDirectoryBrowse_clicked();

    void on_deleteButton_clicked();

    void on_rbRunLocally_clicked();

    void on_rbConnectRemote_clicked();

    void on_advancedCheckbox_clicked();

    void on_enabledCheckbox_clicked();

    void on_hideAppCheckBox_clicked();
public slots:
    void on_appTypeCombo_currentIndexChanged(const QString &arg1);

private:
    Ui::ConfigElementFrame *ui;
};

#endif // CONFIGELEMENTFRAME_H

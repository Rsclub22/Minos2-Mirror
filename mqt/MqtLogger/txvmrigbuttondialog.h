#ifndef TXVMRIGBUTTONDIALOG_H
#define TXVMRIGBUTTONDIALOG_H

#include <QDialog>
#include <QValidator>
#include "voicekeyerbase.h"
#include "rigcontrolcommonconstants.h"

namespace Ui {
class TxVmRigButtonDialog;
}

class CWRigKeyerValidator:public QValidator
{
public:
    CWRigKeyerValidator();
    virtual ~CWRigKeyerValidator() override
    {}

    QValidator::State validate(QString & input, int & /*pos*/) const override;
    void setValidCwCharList(QStringList validCwCharList);

private:
    QStringList validCwCharStrList;
};


class TxVmRigButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmRigButtonDialog(QWidget *parent = nullptr);
    ~TxVmRigButtonDialog();

    void setVmData(VoiceKeyerParams* vmData);
    void setCwMessageTextBoxVisible(bool state);
    void setCwMessageLineEditVisible(bool visible);
    void setSerialMessageTextBoxVisible(bool visible);
    void setVmTypeLabelcwMemType(QString mfg);
    void setDialogForCatPttEom(bool state);


    void setRadioNameLbl(QString radioName);
    void setCwValidatorCwCharList(QString validCwCharList);
    void setCwCharValidator(CWRigKeyerValidator cwCharValidator);
    void setCwValidatorMaxCwMessageLength(int maxNumChars);
    void setCwSupportSpecialChar(bool radioSupportSpecialChar);
    void setSpecialCwCharMap(QMap<QString, QChar> &specialCharMap);
    void setCwSupportCharsGroupBoxVisible(bool visible);
    void setCwSupportSpecialCharsGroupBoxVisible(bool visible);
    void setCwInfoPanelVisible(bool visible);
    void setMaxNumberCwCharactersText(int maxNumCwChars);
private slots:
    void on_okButtonClicked();
    void on_cancelbuttonClicked();

    void onVmRepeatPauseDurEditingFinished();
    void onVmMessageDurEditingFinished();
    void on_txCwMessageEditingFinshed();
    void on_txSerialMessageEditingFinshed();
public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;
private:

    Ui::TxVmRigButtonDialog *ui;
    VoiceKeyerParams* vmData;

    CWRigKeyerValidator cwCharValidator;

    QString validCwCharacterList;
    int maximumNumCwChars = 0;
    bool supportSpecialCwChars = false;
    QStringList supportedCwSpecialCharsList;




    bool validateDur(QString durName, QString dur, int &dur_);
    void doCloseEvent();
    bool checkLengthOfCwMessage(int length);


    void populateInfoPanelSupportedSpecialChars();
    void populateInfoPanelSupportedCwChars(QString validCwCharList);
};




#endif // TXVMRIGBUTTONDIALOG_H

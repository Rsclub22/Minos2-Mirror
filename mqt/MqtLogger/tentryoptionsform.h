#ifndef TENTRYOPTIONSFORM_H
#define TENTRYOPTIONSFORM_H

#include <QDialog>
#include "ContestDetailsTransferObject.h"

class LoggerContestLog;

namespace Ui {
class TEntryOptionsForm;
}

class TOptionFrame;
class TEntryOptionsForm : public QDialog
{
    Q_OBJECT
    void doCloseEvent();

public:
    explicit TEntryOptionsForm(QWidget* Owner, QSharedPointer<ContestDetailsTransferObject> ,
                               LoggerContestLog *inputContest, bool minosSave, bool sendEntry );
    ~TEntryOptionsForm() override;

    QString doFileSave( );
private slots:
    void on_CloseButton_clicked();

    void on_CancelButton_clicked();

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::TEntryOptionsForm *ui;

    QSharedPointer<ContestDetailsTransferObject> ct;
    LoggerContestLog *inputContest = nullptr;

    bool minosSave;
    bool sendEntry;
    void getContestOperators();
};

#endif // TENTRYOPTIONSFORM_H

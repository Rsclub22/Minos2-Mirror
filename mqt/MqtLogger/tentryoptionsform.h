#ifndef TENTRYOPTIONSFORM_H
#define TENTRYOPTIONSFORM_H

#include "base_pch.h"

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
    explicit TEntryOptionsForm( QWidget* Owner, LoggerContestLog * , bool minosSave );
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

    void FormShow( );

    LoggerContestLog * ct;

    ExportType expformat;
    bool minosSave;
    void getContestOperators();
};

#endif // TENTRYOPTIONSFORM_H

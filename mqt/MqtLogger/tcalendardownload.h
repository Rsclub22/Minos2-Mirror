#ifndef TCALENDARDOWNLOAD_H
#define TCALENDARDOWNLOAD_H

#include <QTimer>
#include <QDialog>

namespace Ui {
class TCalendarDownload;
}

class TCalendarDownload : public QDialog
{
    Q_OBJECT

public:
    explicit TCalendarDownload(QWidget *parent = nullptr);
    ~TCalendarDownload();

private:
    Ui::TCalendarDownload *ui;
    QTimer formShowTimer;

    void downloadFiles();

private slots:
    void on_formShown();

};

#endif // TCALENDARDOWNLOAD_H

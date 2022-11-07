#ifndef CHECKUPDATES_H
#define CHECKUPDATES_H

#include <QDialog>

namespace Ui {
class CheckUpdates;
}

class CheckUpdates : public QDialog
{
    Q_OBJECT

public:
    explicit CheckUpdates(QWidget *parent = nullptr);
    ~CheckUpdates();

private slots:
    void on_closeButton_clicked();

    void on_sourceforgeButton_clicked();

private:
    Ui::CheckUpdates *ui;
    void doCloseEvent();
    QString downloadFile();
    QString getString(QJsonObject o, QString key, QString def);
};

#endif // CHECKUPDATES_H

#ifndef WSJTXCONFIGURE_H
#define WSJTXCONFIGURE_H

#include <QDialog>

namespace Ui {
class WsjtxConfigure;
}

class WsjtxConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit WsjtxConfigure(QWidget *parent = nullptr);
    ~WsjtxConfigure();

private slots:
    void on_OKButton_clicked();

    void on_CancelButton_clicked();

    void on_alltxtBrowseButton_clicked();

private:
    Ui::WsjtxConfigure *ui;
    void doClose();
};

#endif // WSJTXCONFIGURE_H

#ifndef WSJTXCONFIGURE_H
#define WSJTXCONFIGURE_H

#include <QFrame>

namespace Ui {
class WsjtxConfigure;
}

class WsjtxConfigure : public QFrame
{
    Q_OBJECT

public:
    explicit WsjtxConfigure(QWidget *parent = nullptr);
    ~WsjtxConfigure();

    void initialise();
    void finalise();

private slots:
    void on_alltxtBrowseButton_clicked();

private:
    Ui::WsjtxConfigure *ui;
};

#endif // WSJTXCONFIGURE_H

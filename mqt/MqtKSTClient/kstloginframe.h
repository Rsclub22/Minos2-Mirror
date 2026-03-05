#ifndef KSTLOGINFRAME_H
#define KSTLOGINFRAME_H

#include <QFrame>

namespace Ui {
class KSTLoginFrame;
}

class KSTLoginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTLoginFrame(QWidget *parent = nullptr);
    ~KSTLoginFrame();

    void on_FontChanged();

    void setLogins(QStringList selections);
    void setLoginTexts(QStringList services);
    void do_logincb_stateChanged();

private slots:
    void logincb_stateChanged(int);
    void on_KSTTestButton_clicked();

private:
    Ui::KSTLoginFrame *ui;
};

#endif // KSTLOGINFRAME_H

#ifndef KSTACTIVECHATSFRAME_H
#define KSTACTIVECHATSFRAME_H

#include <QFrame>

namespace Ui {
class KSTActiveChatsFrame;
}
class QCheckBox;
class QRadioButton;
class KSTActiveChatsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTActiveChatsFrame(QWidget *parent = nullptr);
    ~KSTActiveChatsFrame();

    void on_FontChanged();

    void setActive(int chat);
    void checkActive();
    void setLoginTexts(QStringList services);
    void resetVectors(QCheckBox *cb, int c, QStringList &s, QVector<int> &v, QVector<int> &a);

private slots:
    void activerb_clicked();

private:
    Ui::KSTActiveChatsFrame *ui;
};

#endif // KSTACTIVECHATSFRAME_H

#ifndef STATISTICSDIPLAY_H
#define STATISTICSDIPLAY_H

#include <QDialog>

namespace Ui {
class StatisticsDiplay;
}

class BaseContestLog;

class StatisticsDiplay : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDiplay(BaseContestLog * ct, QWidget *parent = nullptr);
    ~StatisticsDiplay();

private slots:
    void on_CloseButton_clicked();

    void on_RecalcButton_clicked();

private:
    Ui::StatisticsDiplay *ui;

    BaseContestLog * ct = nullptr;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

};

#endif // STATISTICSDIPLAY_H

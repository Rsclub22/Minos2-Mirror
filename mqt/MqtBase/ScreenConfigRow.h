#ifndef SCREENCONFIGROW_H
#define SCREENCONFIGROW_H

#include <QFrame>
#include <QHBoxLayout>

class QScrollArea;
class ScreenConfig;
class ScreenConfigElement;
class MinosSplitter;

namespace Ui {
class ScreenConfigRow;
}

class ScreenConfigRow : public QFrame
{
    Q_OBJECT

    void unsplit();
public:
    int tag = 0;

    Ui::ScreenConfigRow *ui;
    bool selected = false;

    explicit ScreenConfigRow(ScreenConfigElement *parentElement);
    ~ScreenConfigRow() override;

    MinosSplitter *hmsplit = nullptr;
    ScreenConfigElement *parentElement;

    ScreenConfigElement *addLeft(ScreenConfigElement *e);
    void remove(ScreenConfigElement *e);
    ScreenConfigElement * addRight(ScreenConfigElement *e);

    bool checkOk(ScreenConfigElement *e);

protected:
    bool mouseDown = false;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void on_addRowBeforeButton_clicked();

    void on_removeRowButton_clicked();

    void on_addRowAfterButton_clicked();

};

#endif // SCREENCONFIGROW_H

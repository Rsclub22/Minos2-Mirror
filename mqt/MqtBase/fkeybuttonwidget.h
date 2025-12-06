#ifndef FKEYBUTTONWIDGET_H
#define FKEYBUTTONWIDGET_H

#include <QFrame>
#include <QObject>
#include <QPushButton>
#include <QGridLayout>

class FKeyButtonWidget : public QFrame
{
    Q_OBJECT
public:
    explicit FKeyButtonWidget(QWidget *parent = nullptr);

    void setButtonLayout(int numButtons, int columns);
    void setButtonText(int buttonNum, const QString &text);
    void clearAllButtons();
    QPushButton* getButton(int index);
    int getButtonCount() const { return fButtons.size(); }

signals:
    void buttonClicked(int buttonNum);

private:
    QVector<QPushButton*> fButtons;
    QGridLayout *layout;

    void clearButtons();
    void createButtons(int numButtons, int columns);
};

#endif // FKEYBUTTONWIDGET_H

#ifndef QSOTEXTEDITFRAME_H
#define QSOTEXTEDITFRAME_H

#include <QFrame>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>

#include "cutils.h"

class QSOTextEditFrame : public QFrame
{
    Q_OBJECT

    UpperCaseValidator ucValidator;

    QLabel *TextEditlabel = nullptr;
    QLineEdit *TextEditEdit = nullptr;

    bool expert = false;

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

public:
    QSOTextEditFrame(QWidget *parent);
    virtual ~QSOTextEditFrame() override;
    void setup(QString name, QWidget *filterWidget, bool uc = true, bool horizontal = false);
    QLineEdit *getTextEditEdit() const;
    QLabel *getTextEditlabel() const;
    void setWidth(QString);
};

#endif // QSOTEXTEDITFRAME_H

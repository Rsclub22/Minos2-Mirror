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
    QToolButton *clearButton = nullptr;

public:
    QSOTextEditFrame(QWidget *parent);
    virtual ~QSOTextEditFrame() override;
    void setup(QString name, bool uc = true, bool horizontal = false);
    QLineEdit *getTextEditEdit() const;
    QLabel *getTextEditlabel() const;
private slots:
    void onTextEdit_textChanged(const QString &);
    void onClearButtonClicked();
};

#endif // QSOTEXTEDITFRAME_H

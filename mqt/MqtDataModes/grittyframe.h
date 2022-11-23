#ifndef GRITTYFRAME_H
#define GRITTYFRAME_H

#include <QFrame>

namespace Ui {
class GrittyFrame;
}
class QLineEdit;
class QTextEdit;

class GrittyFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GrittyFrame(QWidget *parent, QTextEdit *rxChars, QLineEdit *sendEdit, QString fname);
    ~GrittyFrame();

private:
    Ui::GrittyFrame *ui;
};

#endif // GRITTYFRAME_H

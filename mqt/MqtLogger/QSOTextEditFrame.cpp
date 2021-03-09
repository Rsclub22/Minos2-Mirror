#include "QSOTextEditFrame.h"

QSOTextEditFrame::QSOTextEditFrame(QWidget *parent):QFrame(parent)
{

}

QSOTextEditFrame::~QSOTextEditFrame()
{

}
void QSOTextEditFrame::setup(QString name, QWidget *filterWidget, bool uc, bool horizontal)
{
    setFrameShape(QFrame::NoFrame);
    QVBoxLayout *TextEditLayout = nullptr;
    QHBoxLayout *hb = nullptr;

    if (!horizontal)
    {
        TextEditLayout = new QVBoxLayout();
        TextEditLayout->setSpacing(0);
        TextEditLayout->setContentsMargins(2, 2, 2, 2);
        TextEditLayout->setObjectName(name + "Layout");
    }
    hb = new QHBoxLayout();
    hb->setSpacing(0);
    hb->setContentsMargins(0, 0, 0, 0);

    TextEditlabel = new QLabel(this);
    TextEditlabel->setObjectName(name + "label");
    hb->addWidget(TextEditlabel);

    TextEditEdit = new QLineEdit(this);
    //TextEditEdit->setClearButtonEnabled(true);
    TextEditEdit->setObjectName(name + "Edit");
    connect(TextEditEdit, &QLineEdit::textChanged, this, &QSOTextEditFrame::onTextEdit_textChanged);

    if (horizontal)
    {
        hb->addWidget(TextEditEdit);
    }
    if (!horizontal)
    {
        QSpacerItem *horizontalSpacer = new QSpacerItem(4, 2, QSizePolicy::Expanding, QSizePolicy::Minimum);
        hb->addItem(horizontalSpacer);
    }

    clearButton = new QToolButton(this);
    clearButton->setFocusPolicy(Qt::NoFocus);
    clearButton->setEnabled(false);

    clearButton->setToolTip(tr("Click to clear edit content"));
    connect(clearButton, &QToolButton::clicked, this, &QSOTextEditFrame::onClearButtonClicked);

    hb->addWidget(clearButton);

    if (!horizontal)
    {
        TextEditLayout->addLayout(hb);
        TextEditLayout->addWidget(TextEditEdit);
        setLayout(TextEditLayout);
    }
    else
    {
        setLayout(hb);
    }

    if (uc)
    {
        TextEditEdit->setValidator(&ucValidator);
    }
    TextEditEdit->installEventFilter(filterWidget);

    setFocusPolicy(Qt::NoFocus);
}
QLineEdit *QSOTextEditFrame::getTextEditEdit() const
{
    return TextEditEdit;
}

QLabel *QSOTextEditFrame::getTextEditlabel() const
{
    return TextEditlabel;
}
void QSOTextEditFrame::onTextEdit_textChanged(const QString &arg)
{

    if (arg.isEmpty())
    {
        clearButton->setText("");
        clearButton->setEnabled(false);
    }
    else
    {
        if (!TextEditEdit->isReadOnly())
        {
            clearButton->setText("x");
            clearButton->setEnabled(true);
        }
    }
}
void QSOTextEditFrame::onClearButtonClicked()
{
    TextEditEdit->clear();
    TextEditEdit->setFocus();
}

#include "ContestApp.h"
#include "QSOTextEditFrame.h"

QSOTextEditFrame::QSOTextEditFrame(QWidget *parent):QFrame(parent)
{
    TContestApp::getContestApp() ->getBoolDisplayProfile( edpExpertMode, expert );

}

QSOTextEditFrame::~QSOTextEditFrame()
{

}
void QSOTextEditFrame::setup(QString name, QWidget *filterWidget, bool uc)
{
    setFrameShape(QFrame::NoFrame);
    QVBoxLayout *TextEditLayout = nullptr;
    QHBoxLayout *hb = nullptr;
    QHBoxLayout *thb = nullptr;

    TextEditLayout = new QVBoxLayout();
    TextEditLayout->setSpacing(0);
    TextEditLayout->setContentsMargins(2, 2, 2, 2);
    TextEditLayout->setObjectName(name + "Layout");

    hb = new QHBoxLayout();
    hb->setSpacing(0);
    hb->setContentsMargins(0, 0, 0, 0);

    TextEditlabel = new QLabel(this);
    TextEditlabel->setObjectName(name + "label");
    hb->addWidget(TextEditlabel);

    thb = new QHBoxLayout();
    thb->setSpacing(0);
    thb->setContentsMargins(0, 0, 0, 0);

    TextEditEdit = new QLineEdit(this);
    TextEditEdit->setObjectName(name + "Edit");
    TextEditEdit->setClearButtonEnabled(true);
    thb->addWidget(TextEditEdit);

    QSpacerItem *horizontalSpacer = new QSpacerItem(4, 2, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hb->addItem(horizontalSpacer);

    TextEditLayout->addLayout(hb);
    TextEditLayout->addLayout(thb);
    setLayout(TextEditLayout);

    if (uc)
    {
        TextEditEdit->setValidator(&ucValidator);
    }
    TextEditEdit->installEventFilter(filterWidget);

    setFocusPolicy(Qt::NoFocus);

    TextEditEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    TextEditlabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
}

QLineEdit *QSOTextEditFrame::getTextEditEdit() const
{
    return TextEditEdit;
}

QLabel *QSOTextEditFrame::getTextEditlabel() const
{
    return TextEditlabel;
}

void QSOTextEditFrame::setWidth(QString s)
{
    QFont lf = TextEditlabel->font();
    QFontMetrics lfm(lf);

    QString l = TextEditlabel->text();
    int b = l.indexOf("<b>");
    if (b >= 0)
    {
        l = l.right(l.length() - (b + 3));
    }
    int tell = lfm.boundingRect(l + "W").width() * 8/7; // allowance for bold

    TextEditlabel->setMaximumWidth(tell);
    TextEditlabel->setMinimumWidth(tell);

    QFont ef = TextEditEdit->font();
    QFontMetrics efm(ef);

    int teel = efm.boundingRect(s + "WWW").width();    //allow for close box

    TextEditEdit->setMaximumWidth(teel);
    TextEditEdit->setMinimumWidth(teel);

    QSize sh = minimumSizeHint();
    setMinimumSize(sh);
    setMaximumSize(sh);
}

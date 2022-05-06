#include "ContestApp.h"
#include "QSOTextEditFrame.h"

QSOTextEditFrame::QSOTextEditFrame(QWidget *parent):QFrame(parent)
{
    TContestApp::getContestApp() ->getBoolDisplayProfile( edpExpertMode, expert );

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
    TextEditEdit->setObjectName(name + "Edit");
    TextEditEdit->setClearButtonEnabled(true);
    TextEditEdit->installEventFilter(this);

    if (horizontal)
    {
        hb->addWidget(TextEditEdit);
    }
    if (!horizontal)
    {
        QSpacerItem *horizontalSpacer = new QSpacerItem(4, 2, QSizePolicy::Expanding, QSizePolicy::Minimum);
        hb->addItem(horizontalSpacer);
    }

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

    TextEditEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    TextEditlabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
}
bool QSOTextEditFrame::eventFilter(QObject *obj, QEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)

    if (obj == TextEditEdit)
    {
        // Fix placeholder colour set incorrectly (QTBUG-95297)
        if (event->type() == QEvent::PaletteChange)
        {
            auto pal = palette();
            for (auto role : { QPalette::Active, QPalette::Disabled, QPalette::Inactive })
            {
                int palalpha = pal.color(role, QPalette::PlaceholderText).alpha();
                if (palalpha == 255)
                {
                    auto col = pal.color(role, QPalette::Text);
                    col.setAlpha(128);
                    pal.setColor(role, QPalette::PlaceholderText, col);
                    setPalette(pal);
                }
            }

            //return true;
        }
    }
#endif
    return false;
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int tell = lfm.horizontalAdvance(l + "W");
#else
    int tell = lfm.width(l + "W");
#endif

    TextEditlabel->setMaximumWidth(tell);
    TextEditlabel->setMinimumWidth(tell);

    QFont ef = TextEditEdit->font();
    QFontMetrics efm(ef);
    int editHeight = efm.height();


#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int teel = efm.horizontalAdvance(s);
#else
    int teel = efm.width(s);
#endif

    teel += editHeight; // allow for clear button

    teel = std::max(teel, tell);

    TextEditEdit->setMaximumWidth(teel);
    TextEditEdit->setMinimumWidth(teel);

    QSize sh = minimumSizeHint();
    setMinimumSize(sh);
    setMaximumSize(sh);
}

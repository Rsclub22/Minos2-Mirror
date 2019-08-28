#include "base_pch.h"
#include "ContestApp.h"
#include "contest.h"
#include "htmldelegate.h"
#include "cutils.h"
#include "locTreeFrame.h"
#include "ui_locTreeFrame.h"

LocTreeFrame::LocTreeFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LocTreeFrame), ct(nullptr)
{
    ui->setupUi(this);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate> (new HtmlDelegate(1.0, lcf/100.0));
    ui->LocTree->setItemDelegate(delegate.data());
}
LocTreeFrame::~LocTreeFrame()
{
    delete ui;
    ct = nullptr;
}
void LocTreeFrame::setContest(BaseContestLog *contest)
{
    ct = contest;

    if (ct)
    {
        reInitialiseLocators();
    }
}

void LocTreeFrame::reInitialiseLocators()
{
    ui->LocTree->clear();

    if (!ct)
        return;

    for (int k = 0; k < ct->locs.llist.size(); k++)
    {
        QSharedPointer<LocSquare> l = ct->locs.itemAt(k);
        if (!l)
            break;

        if (l->isClear())
            continue;
        QString locStart = l ->loc;

        QTreeWidgetItem *it = new QTreeWidgetItem(ui->LocTree);
        it->setText(0, locStart);
        it->setExpanded(true);

        for (int j = 0; j < 10; j++)
        {
            QString dispLine;
            for (int i = 0; i < 10; i++)
            {
                LocCount *lc = l ->map( j * 10 + i );
                QString disp = QString("%1").arg(j * 10 + i, 2, 10, QChar('0'));

                if ( lc && (lc->UKLocCount || lc->nonUKLocCount))
                {
                    if (ct->usesBonus.getValue())
                    {
                        QColor multhighlight = Qt::black;
                        switch (ct->getSquareBonus(locStart + disp))
                        {
                        case 500:  //blue
                            multhighlight = Qt::blue;
                            break;
                        case 1000: //green
                            multhighlight = Qt::darkGreen;
                            break;
                        case 2000: //red
                            multhighlight = Qt::red;
                            break;
                        }

                        dispLine += HtmlFontColour(multhighlight) + "<b>" + disp + "</b>" + " (" + QString::number(lc->UKLocCount + lc->nonUKLocCount) + ") ";
                    }
                    else
                    {
//                        dispLine += disp + " (" + QString::number(lc->UKLocCount)
//                                    + (lc->nonUKLocCount?("/" + QString::number(lc->nonUKLocCount)):QString("")) + ") ";
                        dispLine += disp + " (" + QString::number(lc->UKLocCount + lc->nonUKLocCount) + ") ";
                    }

                }
            }
            if (dispLine.size())
            {
                QTreeWidgetItem *treeItem = new QTreeWidgetItem();

                treeItem->setText(0, dispLine);

                it->addChild(treeItem);
            }
        }
    }
}


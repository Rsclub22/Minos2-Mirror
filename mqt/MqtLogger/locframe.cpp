#include "base_pch.h"
#include "ContestApp.h"
#include "cutils.h"
#include "contest.h"
#include "htmldelegate.h"
#include "MinosLoggerEvents.h"

#include "locframe.h"
#include "ui_locframe.h"

static QString lConv(const QString &tlsq, int col, int row)
{
    //convert column and row to an actual display
   char cvxl = tlsq[0].toLatin1();
   char cvyl = tlsq[1].toLatin1();
   char cvxn = tlsq[2].toLatin1();
   char cvyn = tlsq[3].toLatin1();

   for ( int i = 0; i < col; i++ )
   {
      char cv = ++cvxn;

      if ( cv > '9' )
      {
         cvxn = '0';
         ++cvxl;
      }
   }

   for ( int i = 0; i < row; i++ )
   {
      char cv = --cvyn;

      if ( cv < '0' )
      {
         cvyn = '9';
         --cvyl;
      }
   }

   QString res = QString(cvxl) + cvyl + cvxn + cvyn;
   return res;
}
static QString l_add(const QString &sq, int x, int y)
{
   char cvxl = sq[0].toLatin1();
   char cvyl = sq[1].toLatin1();
   char cvxn = sq[2].toLatin1();
   char cvyn = sq[3].toLatin1();

   for ( int i = 0; i < x; i++ )
   {
      char cv = ++cvxn;

      if ( cv > '9' )
      {
         cvxn = '0';
         ++cvxl;
      }
   }

   for ( int i = 0; i < y; i++ )
   {
      char cv = ++cvyn;

      if ( cv > '9' )
      {
         cvyn = '0';
         ++cvyl;
      }
   }

   QString res = QString(cvxl) + cvyl + cvxn + cvyn;
   return res;
}
static QString l_sub(const QString &sq, int x, int y)
{
    char cvxl = sq[0].toLatin1();
    char cvyl = sq[1].toLatin1();
    char cvxn = sq[2].toLatin1();
    char cvyn = sq[3].toLatin1();

    for ( int i = 0; i < x; i++ )
    {
      char cv = --cvxn;

      if ( cv < '0' )
      {
         cvxn = '9';
         --cvxl;
      }
   }

   for ( int i = 0; i < y; i++ )
   {
      char cv = --cvyn;

      if ( cv < '0' )
      {
            cvyn= '9';
            --cvyl;
        }
    }

   QString res = QString(cvxl) + cvyl + cvxn + cvyn;
   return res;
}
bool oldAL (QString tl, QString oldTl, bool &above, bool &left)
{
    // is oldTl above/left of tl?

    above = false;
    left = false;

    char oldxl = oldTl[0].toLatin1();
    char oldyl = oldTl[1].toLatin1();
    char oldxn = oldTl[2].toLatin1();
    char oldyn = oldTl[3].toLatin1();

    char xl = tl[0].toLatin1();
    char yl = tl[1].toLatin1();
    char xn = tl[2].toLatin1();
    char yn = tl[3].toLatin1();

    //  IO91 JO01 JO11
    //  IO90 JO00 JO10
    //  IN99 JN09 JN19
    //  IN98 JN08 JN18

    if (oldxl < xl) // xl increments to E
    {
        left = true;
    }
    else if (oldxl == xl)
    {
        if (oldxn < xn) // xn increments to E
            left = true;
    }

    if (oldyl > yl)  // yl increments to N
    {
        above = true;
    }
    else if (oldyl == yl)
    {
        if (oldyn > yn) // yn increments to S
            above = true;
    }
    return left ||  above;
}


LocFrame::LocFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LocFrame), ct(nullptr)
{
    ui->setupUi(this);

    currentCentre = "IO91";

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate> (new HtmlDelegate(1.0, lcf/100.0));
    ui->LocView->setItemDelegate(delegate.data());

    model = new LocGridModel();
    model->delegate = delegate;
    ui->LocView->setModel(model);

    connect(ui->LocView, SIGNAL(minosViewScrolled()), this, SLOT(on_minosViewScrolled()));
}

LocFrame::~LocFrame()
{
    delete ui;
    ct = nullptr;
    delete model;
}
void LocFrame::setContest(BaseContestLog *contest)
{
    model->ct = contest;
    ct = contest;
    if (ct)
    {
        currentCentre = ct->myloc.getLoc().left(4);
        model->myLoc = currentCentre;

        reInitialiseLocators();
    }
}
void LocFrame::reInitialiseLocators()
{
    QString oldTl = model->getTl();
    int oldRows = model->rowCount();
    int oldCols = model->columnCount();

    model->beginReset();
    model->locMap.clear();

    if (!ct)
        return;

    //initialise these to a range round the contest location
    QString ctLoc = ct->myloc.getLoc();

    QString ctLocN = l_add(ctLoc, 0, 5);
    QString NLoc = QString(ctLocN[1]) + ctLocN[3];

    QString ctLocS = l_sub(ctLoc, 0, 5);
    QString SLoc = QString(ctLocS[1]) + ctLocS[3];

    QString ctLocW = l_sub(ctLoc, 3, 0);
    QString WLoc = QString(ctLocW[0]) + ctLocW[2];

    QString ctLocE = l_add(ctLoc, 3, 0);
    QString ELoc = QString(ctLocE[0]) + ctLocE[2];


    for (int k = 0; k < ct->locs[ct->currentBand.getValue()].llist.size(); k++)
    {
        QString locStart = ct->locs[ct->currentBand.getValue()].itemAt(k) ->loc;

        for (int j = 0; j < 10; j++)
        {
            for (int i = 0; i < 10; i++)
            {
                LocCount *lc = ct->locs[ct->currentBand.getValue()].itemAt(k) ->map( j * 10 + i );
                QString disp = QString("%1").arg(j * 10 + i, 2, 10, QChar('0'));

                if ( lc && (lc->UKLocCount || lc->nonUKLocCount))
                {
                    model->locMap[locStart + disp] = lc;

                    QString col = QString(locStart[0]) + disp[0];
                    QString row = QString(locStart[1]) + disp[1];

                    if (row < SLoc || SLoc.isEmpty())
                        SLoc = row;

                    if (row > NLoc || NLoc.isEmpty())
                        NLoc = row;

                    if (col < WLoc || WLoc.isEmpty())
                        WLoc = col;

                    if (col > ELoc || ELoc.isEmpty())
                        ELoc = col;
                }
            }
        }
    }

    int rows = (NLoc[0].toLatin1() - SLoc[0].toLatin1()) * 10 + (NLoc[1].toLatin1() - SLoc[1].toLatin1()) + 1;

    int cols = (ELoc[0].toLatin1() - WLoc[0].toLatin1()) * 10 + (ELoc[1].toLatin1() - WLoc[1].toLatin1()) + 1;

    QString tl = QString(WLoc[0]) + NLoc[0] + WLoc[1] + NLoc[1];

    bool xxabove;
    bool xxleft;
    bool xxl;
    xxl = oldAL(tl, oldTl, xxabove, xxleft);
    if (xxl)
    {

    }

    model->rows = rows;
    model->cols = cols;
    model->setTl(tl);

    model->endReset();

    // don't resize earlier, or there won't be ANY DATA TO RESIZE TO...
    ui->LocView->resizeColumnsToContents();
    ui->LocView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for(int i = 0; i < model->rowCount(); i++)
    {
        for(int j = 0; j < model->columnCount(); j++)
        {
            const QModelIndex index = model->index(i, j);
            QString cell = model->data(index, Qt::UserRole).toString();
            if (cell == currentCentre)
            {
                ui->LocView->scrollTo(index, QAbstractItemView::PositionAtCenter);
            }
        }
    }
}


void LocFrame::on_minosViewScrolled()
{
    QModelIndex index = ui->LocView->indexAt(ui->LocView->rect().center());
    currentCentre = ui->LocView->model()->data(index, Qt::UserRole).toString();
}
void LocFrame::on_LocView_clicked(const QModelIndex &index)
{
    if (!ct || model->getTl().isEmpty())
        return;

    QString disp = lConv(model->getTl(), index.column(), index.row());

    QString brgbuff;

    double lon = 0.0;
    double lat = 0.0;

    Locator loc;
    loc.setLoc(disp);

    int lres = lonlat( loc.getLoc(), lon, lat, true );
    if ( lres == LOC_OK )
    {
       int brg;
       double dist;

       ct->disbeara( lon, lat, dist, brg );

       int offset = ct->bearingOffset.getValue();
       brgbuff = QString( "%1").arg( varBrg(brg + offset), 3);
    }

    MinosLoggerEvents::SendBrgStrToRot(brgbuff);
}

void LocFrame::on_LocView_doubleClicked(const QModelIndex &index)
{
    model->beginReset();
    QString tl = model->getTl();
    QString oldTl = tl;
    if (index.row() == 0)
    {
        tl = l_add(tl, 0, 5);
        model->setTl(tl);
        model->rows += 5;
    }
    if (index.column() == 0)
    {
        tl = l_sub(tl, 5, 0);
        model->setTl(tl);
        model->cols += 5;
    }
    if (index.row() == model->rowCount() - 1)
    {
        model->rows += 5;
    }
    if (index.column() == model->columnCount() - 1)
    {
        model->cols += 5;
    }
    model->endReset();
    ui->LocView->resizeColumnsToContents();
    ui->LocView->resizeRowsToContents();
}

LocGridModel::LocGridModel():ct(nullptr), rows(10), cols(10)
{}
LocGridModel::~LocGridModel()
{
}

void LocGridModel::beginReset()
{
    beginResetModel();
}

void LocGridModel::endReset()
{
    endResetModel();
}

QVariant LocGridModel::data( const QModelIndex &index, int role ) const
{
    if (!ct || tlLoc.isEmpty())
        return QVariant();

    QString disp = lConv(tlLoc, index.column(), index.row());

    if (role == Qt::UserRole)
        return disp;

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignLeft; // but HtmlDelegate overrides

    if (role == Qt::DisplayRole)
    {
        QMap<QString, LocCount * >::const_iterator lci = locMap.find(disp);

        if (disp == myLoc)
        {
            disp = HtmlFontColour(Qt::white) + disp;
        }
        if (lci != locMap.end())
            disp = /*HtmlFontColour(multhighlight) +*/ "<b>" + disp ;
        return disp;
    }

    if (role == Qt::BackgroundRole)
    {
        QColor multhighlight = Qt::darkGray;
            if (ct->usesBonus.getValue())
        {
            switch (ct->getSquareBonus(disp))
            {
            case 500:  //blue
                multhighlight = Qt::blue;
                break;
            case 1000: //green
                multhighlight = Qt::green;
                break;
            case 2000: //red
                multhighlight = Qt::red;
                break;
            }
        }
        QMap<QString, LocCount * >::const_iterator lci = locMap.find(disp);
        if (lci == locMap.end())
        {
            return multhighlight.lighter(180);
        }
        else
        {
            return multhighlight.lighter(140);
        }
    }
    return QVariant();
}
QVariant LocGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{
    if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
    {
        if (delegate)
        {
            QString s = data(index(section, 0), Qt::DisplayRole).toString();
            QSize r = delegate->docSize(s);
            return r;
        }
    }
    return QVariant();
}
QModelIndex LocGridModel::index( int row, int column, const QModelIndex &/*parent*/) const
{
    if ( row < 0 || row >= rowCount()  )
        return QModelIndex();

    if ( column < 0 || column >= columnCount()  )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex LocGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int LocGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
//    return 10;
    return rows;
}

int LocGridModel::columnCount( const QModelIndex &/*parent*/ ) const
{
//    return 10;
    return cols;
}


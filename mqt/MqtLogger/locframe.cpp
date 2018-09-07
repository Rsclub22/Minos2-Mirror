#include "base_pch.h"
#include "contest.h"
#include "locframe.h"
#include "ui_locframe.h"
#include "htmldelegate.h"

static QString lConv(const QString &tlsq, int col, int row)
{
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


LocFrame::LocFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LocFrame), ct(nullptr)
{
    ui->setupUi(this);

    currentCentre = "IO91";

    ui->LocView->setItemDelegate(new HtmlDelegate);

    model = new LocGridModel();
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
        currentCentre = ct->myloc.loc.getValue().left(4);

        reInitialiseLocators();
    }
}

void LocFrame::reInitialiseLocators()
{
    model->beginReset();
    model->locMap.clear();

    if (!ct)
        return;

    //initialise these to a range round the contest location
    QString ctLoc = ct->myloc.loc.getValue();

    QString ctLocN = l_add(ctLoc, 0, 5);
    QString NLoc = QString(ctLocN[1]) + ctLocN[3];

    QString ctLocS = l_sub(ctLoc, 0, 5);
    QString SLoc = QString(ctLocS[1]) + ctLocS[3];

    QString ctLocW = l_sub(ctLoc, 3, 0);
    QString WLoc = QString(ctLocW[0]) + ctLocW[2];

    QString ctLocE = l_add(ctLoc, 3, 0);
    QString ELoc = QString(ctLocE[0]) + ctLocE[2];

    for (int k = 0; k < ct->locs.llist.size(); k++)
    {
        QString locStart = ct->locs.itemAt(k) ->loc;

        for (int j = 0; j < 10; j++)
        {
            for (int i = 0; i < 10; i++)
            {
                LocCount *lc = ct->locs.itemAt(k) ->map( j * 10 + i );
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
    model->rows = rows;

    int cols = (ELoc[0].toLatin1() - WLoc[0].toLatin1()) * 10 + (ELoc[1].toLatin1() - WLoc[1].toLatin1()) + 1;
    model->cols = cols;

    QString tl = QString(WLoc[0]) + NLoc[0] + WLoc[1] + NLoc[1];
    model->setTl(tl);

    model->endReset();

    // don't resize earlier, or there won't be ANY DATA TO RESIZE TO...
   // ui->LocView->resizeColumnsToContents();
   // ui->LocView->resizeRowsToContents();

    // We seem to have to use the application font, as Qt can give info
    // on the wrong font if we use the widgets font
    QFontMetricsF fm(QApplication::font());
    int width=static_cast<int>((fm.width("MM80") * 5)/4.0);
    int height=static_cast<int>((fm.height() * 6)/4.0);

    ui->LocView->horizontalHeader()->setDefaultSectionSize(width);
    ui->LocView->verticalHeader()->setDefaultSectionSize(height);

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

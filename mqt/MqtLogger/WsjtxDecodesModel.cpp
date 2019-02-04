#include "base_pch.h"

#include <QStandardItem>

#include "WsjtxRadio.hpp"
#include "cutils.h"

#include "WsjtxDecodesModel.hpp"
/*
  We want:

  Message
  "Answerable" - i.e. CQ or call to me
  potential points difference
  distance
  bearing



  click to populate rotator
  double click to reply

  set "auto" to reply to the "best available" over the minimum distance


  reply to CQ (who can be working someone)
           Tail ender (calls me while I'm working someone else) - pref at 73 or RR73 stage
           one of several responses to my CQ

  Then let WSJT-X "auto seq" manage the QSO

  Just because I am CQing doesn't mean I have to take replies - I can respond to a CQer instead

  All this has to happen after decode transitions true to false, at which point we have all the decodes

  Do we need a whitelist(work if available)/blacklist(ignore)?

  What about EU contest mode? Can  we tell? and so ignore/not ignore as appropriate.

  We need to "age" decodes ?? no more than 10 decode periods?

  What about the oher modes?

  */
namespace
{
class DecodeHeading
{
public:
    QString text;
    Qt::AlignmentFlag alignment;
};

DecodeHeading const headings[dcMaxVal] = {
    {QT_TRANSLATE_NOOP ("DecodesModel", "Client"), Qt::AlignRight},

    {QT_TRANSLATE_NOOP ("DecodesModel", "Time"), Qt::AlignRight},
    {QT_TRANSLATE_NOOP ("DecodesModel", "Snr"), Qt::AlignRight},
    {QT_TRANSLATE_NOOP ("DecodesModel", "DT"), Qt::AlignRight},
    {QT_TRANSLATE_NOOP ("DecodesModel", "DF"), Qt::AlignRight},
    {QT_TRANSLATE_NOOP ("DecodesModel", "Md"),Qt::AlignHCenter},
    {QT_TRANSLATE_NOOP ("DecodesModel", "Confidence"),Qt::AlignHCenter},
    {QT_TRANSLATE_NOOP ("DecodesModel", "Live"),Qt::AlignHCenter},

    {QT_TRANSLATE_NOOP ("DecodesModel", "Seq"),Qt::AlignHCenter},
    {QT_TRANSLATE_NOOP ("DecodesModel", "points"),Qt::AlignHCenter},
    {QT_TRANSLATE_NOOP ("DecodesModel", "bearing"),Qt::AlignHCenter},
    {QT_TRANSLATE_NOOP ("DecodesModel", "distance"),Qt::AlignHCenter},

    {QT_TRANSLATE_NOOP ("DecodesModel", "From call"), Qt::AlignLeft},
    {QT_TRANSLATE_NOOP ("DecodesModel", "From grid"), Qt::AlignLeft},
    {QT_TRANSLATE_NOOP ("DecodesModel", "To Call"), Qt::AlignLeft},
    {QT_TRANSLATE_NOOP ("DecodesModel", "To Grid"), Qt::AlignLeft},
    {QT_TRANSLATE_NOOP ("DecodesModel", "Best"), Qt::AlignLeft},

    {QT_TRANSLATE_NOOP ("DecodesModel", "Message"), Qt::AlignLeft}
};

QRegularExpression cq_re {"(CQ|CQDX|QRZ)[^A-Z0-9/]+"};

QString confidence_string (bool low_confidence)
{
    return low_confidence ? QT_TRANSLATE_NOOP ("DecodesModel", "low") : QT_TRANSLATE_NOOP ("DecodesModel", "high");
}

QString live_string (bool off_air)
{
    return off_air ? QT_TRANSLATE_NOOP ("DecodesModel", "no") : QT_TRANSLATE_NOOP ("DecodesModel", "yes");
}

}

DecodesModel::DecodesModel ()
    : QAbstractItemModel (), rx_df_ (-1)
{
    int column {0};
    for (auto const& heading : headings)
    {
        setHeaderData (column++, Qt::Horizontal, heading.text);
    }
}

void DecodesModel::add_decode ()
{
    beginResetModel();
    endResetModel();
}

void DecodesModel::de_call (QString const& call)
{
    // sets up my call
    if (call != call_)
    {
        beginResetModel ();
        if (call.size ())
        {
            base_call_re_.setPattern ("[^A-Z0-9]*" + Radio::base_callsign (call) + "[^A-Z0-9]*");
        }
        else
        {
            base_call_re_.setPattern (QString {});
        }
        call_ = call;
        endResetModel ();
    }
}

void DecodesModel::rx_df (int df)
{
    // sets up my delta frequency
    if (df != rx_df_)
    {
        beginResetModel ();
        rx_df_ = df;
        endResetModel ();
    }
}
QVariant DecodesModel::data (QModelIndex const& index, int role) const
{

    if (role == Qt::BackgroundRole)
    {
        switch (index.column ())
        {
        case dcMessage:                 // message
        {
            auto message = data (index).toString ();
            if (base_call_re_.pattern ().size ()
                    && message.contains (base_call_re_))
            {
                // my call in message - colour red(ish)
                return QColor {255,200,200};
            }
            if (message.contains (cq_re))
            {
                // CQ call in message - colour green(ish)
                return QColor {200, 255, 200};
            }
        }
            break;

        case dcDF:                 // DF
            if (qAbs (data (index).toInt () - rx_df_) <= 10)
            {
                // near my freq  - colour red(ish)
                return QColor {255, 200, 200};
            }
            break;

        case dcTime:
            {
                QString secs = messages->at(index.row()).time.toString ("ss");
                if (secs == "00")
                {
                    return QColor(Qt::red).lighter();
                }
                else if (secs == "15")
                {
                    return QColor(Qt::green).lighter();
                }
                else if (secs == "30")
                {
                    return QColor(Qt::blue).lighter();
                }
                else if (secs == "45")
                {
                    return QColor(Qt::yellow).lighter();
                }
            }
            break;

        default:
            break;
        }
    }
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case dcId:
            return messages->at(index.row()).id;
        case dcTime:
            return messages->at(index.row()).time.toString ("hh:mm:ss");
        case dcSnr:
            return QString::number(messages->at(index.row()).snr);
        case dcDT:
            return QString::number (static_cast<double>( messages->at(index.row()).delta_time));
        case dcDF:
            return QString::number(messages->at(index.row()).delta_frequency);
        case dcMd:
            return messages->at(index.row()).mode;
        case dcConfidence:
            return  confidence_string (messages->at(index.row()).low_confidence) ;
        case dcLive:
            return live_string (messages->at(index.row()).off_air);
        case dcSeq:
            return messages->at(index.row()).getMStage();

        case dcPoints:
            return QString::number(messages->at(index.row()).points);
        case dcBearing:
            return QString::number(messages->at(index.row()).bearing);
        case dcDistance:
            return QString::number(messages->at(index.row()).distance);

        case dcFromCall:
            return messages->at(index.row()).fromCall.realCall;
        case dcFromGrid:
            return messages->at(index.row()).fromGrid.loc.getValue();
        case dcToCall:
            return messages->at(index.row()).toCall.realCall;
        case dcToGrid:
            return messages->at(index.row()).toGrid.loc.getValue();

        case dcBest:
            return messages->at(index.row()).best?"Best":"";

        case dcMessage:
            return messages->at(index.row()).message;

        }
    }

    return QVariant();
}

QVariant DecodesModel::headerData (int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString h = headings[ section ].text;
        return h;
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return headings[ section ].alignment;
    }
    else if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
    {
        if (delegate)
        {
            QString s = data(index(section, 0), Qt::DisplayRole).toString();
            QSize r = delegate->docSize(s);
            r.setWidth(0);
            return r;
        }
    }
    return QVariant();

}
QModelIndex DecodesModel::index( int row, int column, const QModelIndex &parent) const
{
    if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex DecodesModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int DecodesModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    return messages->size();
}

int DecodesModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return  dcMaxVal;
}
void DecodesModel::clear()
{
    beginResetModel();
    messages->clear();
    endResetModel();
}

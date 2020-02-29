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

DecodeHeading const DecodesModel::headings[dcMaxVal] = {
    {QT_TR_NOOP ("Client"), Qt::AlignLeft},

    {QT_TR_NOOP ("Time"), Qt::AlignLeft},
    {QT_TR_NOOP ("Snr"), Qt::AlignLeft},
    {QT_TR_NOOP ("DT"), Qt::AlignLeft},
    {QT_TR_NOOP ("DF"), Qt::AlignLeft},
    {QT_TR_NOOP ("Md"),Qt::AlignLeft},
    {QT_TR_NOOP ("Confidence"),Qt::AlignLeft},
    {QT_TR_NOOP ("Live"),Qt::AlignLeft},

    {QT_TR_NOOP ("Seq"),Qt::AlignLeft},
    {QT_TR_NOOP ("points"),Qt::AlignLeft},
    {QT_TR_NOOP ("brg"),Qt::AlignLeft},
    {QT_TR_NOOP ("distance"),Qt::AlignLeft},

    {QT_TR_NOOP ("Call"), Qt::AlignLeft},
    {QT_TR_NOOP ("Grid"), Qt::AlignLeft},
    {QT_TR_NOOP ("To Call"), Qt::AlignLeft},
    {QT_TR_NOOP ("To Grid"), Qt::AlignLeft},
    {QT_TR_NOOP ("Best"), Qt::AlignLeft},

    {QT_TR_NOOP ("Message"), Qt::AlignLeft}
};

QRegularExpression cq_re {"(CQ|CQDX|QRZ)[^A-Z0-9/]+"};

QString DecodesModel::confidence_string (bool low_confidence) const
{
    return low_confidence ? tr ("low") : tr ("high");
}

QString DecodesModel::live_string (bool off_air) const
{
    return off_air ? tr ("no") : tr ("yes");
}

DecodesModel::DecodesModel ()
    : QAbstractItemModel (), rx_df_ (-1)
{
    int column {0};
    for (auto const& heading : headings)
    {
        setHeaderData (column++, Qt::Horizontal, tr(heading.text));
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

    const decodeMessage &msg = messages->at(index.row());
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
                if (msg.colOffset == 0)
                {
                    return QColor(Qt::red).lighter();
                }
                else if (msg.colOffset  == 1)
                {
                    return QColor(Qt::green).lighter();
                }
                else if (msg.colOffset  == 2)
                {
                    return QColor(Qt::blue).lighter();
                }
                else if (msg.colOffset  == 3)
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
            return msg.id;
        case dcTime:
            return msg.time.toString ("hh:mm:ss");
        case dcSnr:
            return QString::number(msg.snr);
        case dcDT:
            return QString::number (static_cast<double>( msg.delta_time));
        case dcDF:
            return QString::number(msg.delta_frequency);
        case dcMd:
            return msg.mode;
        case dcConfidence:
            return  confidence_string (msg.low_confidence) ;
        case dcLive:
            return live_string (msg.off_air);
        case dcSeq:
            return msg.getMStage();

        case dcPoints:
        {
            bool highlight = false;

            if (msg.mstage != emsCQ && msg.mstage != emsGrid && msg.mstage != ems73 && msg.mstage != emsRRR )
            {
                return "";
            }
            if (msg.csret == ERR_DUPCS)
            {
                return "(wkd)";
            }
            if ((msg.mstage == ems73 || msg.mstage == emsRRR))
            {
                if (msg.toCall == call_ )
                {
                    return "";
                }
            }
            QString points = QString::number(msg.points);

            if (msg.bonus)
            {
                highlight = true;
                points += " (b+" + QString::number(msg.bonus) + ")";
            }
            if (msg.mults)
            {
                highlight = true;
                points += " (m*" + QString::number(msg.mults) + ")";
            }
            QString pts = (highlight?HtmlFontColour(Qt::red):QString()) + points;
            return  pts;
        }
        case dcBearing:
            if (msg.mstage != emsCQ && msg.mstage != emsGrid && msg.mstage != ems73 && msg.mstage != emsRRR)
            {
                return "";
            }
            if (msg.points == 0)
            {
                return "";
            }
            if (msg.csret == ERR_DUPCS)
            {
                return "";
            }
            return QString::number(msg.bearing);
        case dcDistance:
            return QString::number(msg.distance);

        case dcFromCall:
            return msg.fromCall.realCall;
        case dcFromGrid:
            return msg.fromGrid.loc.getValue();
        case dcToCall:
            return msg.toCall.realCall;
        case dcToGrid:
            return msg.toGrid.loc.getValue();

        case dcBest:
        {
            if (msg.oldmsg)
            {
                return tr("(old)");
            }
            if (msg.best)
            {
                if (msg.autoresp)
                {
                    return tr("Auto");
                }
                return tr("Best");
            }
            return "";
        }
        case dcMessage:
            return escapeXML( msg.message );

        }
    }

    return QVariant();
}

QVariant DecodesModel::headerData (int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString h = tr(headings[ section ].text);
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

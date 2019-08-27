#ifndef WSJTX_UDP_DECODES_MODEL_HPP
#define WSJTX_UDP_DECODES_MODEL_HPP

#include "base_pch.h"
#include <QAbstractItemModel>
#include <QRegularExpression>
#include "htmldelegate.h"
#include "WsjtxDecode.h"


enum DecodeColumns
{
    dcId,
    dcTime, dcSnr, dcDT, dcDF, dcMd, dcConfidence, dcLive,
    dcSeq, dcPoints, dcBearing, dcDistance,
    dcFromCall, dcFromGrid, dcToCall, dcToGrid,
    dcBest,
    dcMessage, dcMaxVal
};

class DecodesModel
  : public QAbstractItemModel
{
  Q_OBJECT

private:
  QString client_id_;
  QString call_;
  QRegularExpression base_call_re_;
  int rx_df_;

public:
  QSharedPointer<HtmlDelegate> delegate;
  QVector<decodeMessage> *messages = nullptr;
  explicit DecodesModel ();

  Q_SLOT void add_decode ();

  void de_call (QString const&);
  void rx_df (int);
  void setId(QString clientId)
  {
      client_id_ = clientId;
  }
  QVariant data (QModelIndex const& proxy_index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData (int section, Qt::Orientation orientation,
                               int role = Qt::DisplayRole) const override;
  QModelIndex index( int row, int column,
                     const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
  QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

  int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
  int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;

  void clear();

};

#endif

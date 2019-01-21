#ifndef WSJTX_UDP_DECODES_MODEL_HPP
#define WSJTX_UDP_DECODES_MODEL_HPP

#include "base_pch.h"
#include <QAbstractItemModel>
#include <QRegularExpression>
#include "htmldelegate.h"
#include "WsjtxDecode.h"


//class QTime;
//class QString;
//class QModelIndex;

//
// Decodes Model - simple data model for all decodes
//
// The model is a basic table with uniform row format. Rows consist of
// QStandardItem instances containing the string representation of the
// column data  and if the underlying  field is not a  string then the
// UserRole+1 role contains the underlying data item.
//
// Three slots  are provided to add  a new decode, remove  all decodes
// for a client  and, to build a  reply to CQ message for  a given row
// which is emitted as a signal respectively.
//
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
  HtmlDelegate *delegate = nullptr;
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

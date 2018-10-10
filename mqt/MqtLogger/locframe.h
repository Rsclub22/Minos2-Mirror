#ifndef LOCFRAME_H
#define LOCFRAME_H

#include "base_pch.h"

namespace Ui {
class LocFrame;
}
class BaseContestLog;
class LocCount;

class LocGridModel: public QAbstractItemModel
{
        QString tlLoc;
    public:
        LocGridModel();
        ~LocGridModel() override;

        QString myLoc;

        BaseContestLog *ct;
        int rows;
        int cols;

        QMap<QString, LocCount * > locMap;

        void setTl(const QString &tl)
        {
            tlLoc = tl;
        }

        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;

        void beginReset();
        void endReset() ;
};

class LocFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LocFrame(QWidget *parent = nullptr);
    ~LocFrame();

    void reInitialiseLocators();
    void setContest(BaseContestLog *contest);

private:
    Ui::LocFrame *ui;
    BaseContestLog *ct;
    LocGridModel *model;
    QString currentCentre;

private slots:
    void on_minosViewScrolled();

};

#endif // LOCFRAME_H

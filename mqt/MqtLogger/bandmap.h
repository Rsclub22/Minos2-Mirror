#ifndef BANDMAP_H
#define BANDMAP_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QLocale>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>

class QAbstractItemModel;
class QModelIndex;
class QScrollArea;
class BandmapView;

class Bandmap : public QWidget
{
    Q_OBJECT
public:
    explicit Bandmap(QWidget *parent = nullptr);

    QAbstractItemModel *getBandmapModel() const { return bandmapModel; }
    void setModel(QAbstractItemModel *model);
    QScrollArea *getBandmapScrollArea() const { return bandmapScrollArea; }

    int getSelectedRow() const { return selectedRow; }
    void setSelectedRow(int row);
    int getSelectedColumn() const { return selectedColumn; }
    void setSelectedColumn(int column);

    void initBandmap(QScrollArea *bMapScrollArea);

    void setFreq(double freq);
    int getScrollViewHeight();
signals:
    void clicked(const QModelIndex&);

public slots:

    void setCurrentIndex(const QModelIndex &index);


private:
    QAbstractItemModel *bandmapModel;
    QScrollArea *bandmapScrollArea;

    QHBoxLayout *bandmapLayout;
    BandmapView *bandmapView;


    int selectedRow;
    int selectedColumn;

};

#endif // BANDMAP_H

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


    int getSelectedRow() const { return selectedRow; }
    void setSelectedRow(int row);
    int getSelectedColumn() const { return selectedColumn; }
    void setSelectedColumn(int column);

    void initBandmap();

    void setFreq(double freq);

    int getBandmapFrameHeight();
    int getBandmapFrameWidth();

signals:
    void clicked(const QModelIndex&);

public slots:

    void setCurrentIndex(const QModelIndex &index);


private:
    QAbstractItemModel *bandmapModel;
    QWidget *bandmapFrame;



    BandmapView *bandmapView;


    int selectedRow;
    int selectedColumn;

};

#endif // BANDMAP_H

#include "MinosTableView.h"
#include <QScrollBar>
#include <QHeaderView>

MinosTableView::MinosTableView(QWidget *parent):QTableView(parent), centreCell(QModelIndex())
{

}

void MinosTableView::scrollContentsBy(int dx, int dy)
{
    QTableView::scrollContentsBy(dx, dy);
    emit minosViewScrolled();
}
void MinosTableView::showEvent(QShowEvent *)
{
    if (centreCell.isValid())
    {
        QModelIndex index(centreCell);
        scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}
void MinosTableView::scrollTo(const QModelIndex &index, ScrollHint /*hint*/)
{
    // check if we really need to do anything
    if (!index.isValid())
        return;

    centreCell = index;
    // this only works properly if we scroll per pixel

    // if we don't check on visible, it all goes wrong when hiding the table
    if (!isVisible())
        return;

//    hint == PositionAtCenter; // we never use hint...

    // Adjust horizontal position

    int viewportWidth = viewport()->width();
    int horizontalPosition = horizontalHeader()->sectionPosition(index.column());
    int horizontalIndex = horizontalHeader()->visualIndex(index.column());
    int cellWidth = horizontalHeader()->sectionSize(index.column());

    if (horizontalScrollMode() == QAbstractItemView::ScrollPerItem)
    {

        int w = viewportWidth / 2;
        int x = cellWidth;
        while (horizontalIndex > 0)
        {
            x += columnWidth(horizontalHeader()->logicalIndex(horizontalIndex-1));
            if (x > w)
                break;
            --horizontalIndex;
        }

        horizontalScrollBar()->setValue(horizontalIndex);

    }
    else
    { // ScrollPerPixel
            horizontalScrollBar()->setValue(horizontalPosition - ((viewportWidth - cellWidth) / 2));
    }

    // Adjust vertical position

    int viewportHeight = viewport()->height();
    int verticalPosition = verticalHeader()->sectionPosition(index.row());
    int verticalIndex = verticalHeader()->visualIndex(index.row());
    int cellHeight = verticalHeader()->sectionSize(index.row());

    if (verticalScrollMode() == QAbstractItemView::ScrollPerItem) {

        int h = viewportHeight / 2;
        int y = cellHeight;
        while (verticalIndex > 0) {
            int row = verticalHeader()->logicalIndex(verticalIndex - 1);
            y += verticalHeader()->sectionSize(row);
            if (y > h)
                break;
            --verticalIndex;
        }
        verticalScrollBar()->setValue(verticalIndex);

    }
    else
    { // ScrollPerPixel
        verticalScrollBar()->setValue(verticalPosition - ((viewportHeight - cellHeight) / 2));
    }

    update(index);
}

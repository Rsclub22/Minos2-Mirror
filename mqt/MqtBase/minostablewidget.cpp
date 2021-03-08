/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "base_pch.h"
#include "minostablewidget.h"

#include <QScrollBar>
#include <QHeaderView>


MinosTableWidget::MinosTableWidget(QWidget *parent):QTableView(parent)
{
      statusSortButton = new QToolButton(this);
      resizeTimer = new QTimer(this);
      connect(resizeTimer, &QTimer::timeout, this, &MinosTableWidget::onResizeTimer);
      resizeTimer->start(1000);
}

MinosTableWidget::~MinosTableWidget()
{
    resizeTimer->stop();
    delete statusSortButton;
    statusSortButton = nullptr;
}
void MinosTableWidget::setModel(QAbstractItemModel *model)
{
    init();
    QTableView::setModel(model);
}

void MinosTableWidget::init()
{
      viewport()->stackUnder(statusSortButton);

      statusSortButton->setFocusPolicy(Qt::NoFocus);
      connect(statusSortButton, &QToolButton::clicked, this, &MinosTableWidget::statusClicked);

      statusSortButton->show();

      horizontalHeader()->hideSection(0);

      horizontalHeader()->setVisible(true);
      verticalHeader()->setVisible(true);

}
void MinosTableWidget::statusClicked()
{
    int sortCol = horizontalHeader()->sortIndicatorSection();
    bool sortOrder = horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder;

    if (sortCol != 0)
        sortOrder = false;

    sortOrder = !sortOrder;

    sortByColumn(0, sortOrder?Qt::AscendingOrder:Qt::DescendingOrder);
}

void MinosTableWidget::onResizeTimer()
{
    int width = verticalHeader()->width();
    int height = horizontalHeader()->height();

        // update cornerWidget

    if (statusSortButton)
        statusSortButton->setGeometry(0, 0, width, height);
}

void MinosTableWidget::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    // Modified from the standard so that it DOES scroll...
    // check if we really need to do anything
    if (!index.isValid()
//        || (model()->parent(index) != d->root)
//        || isRowHidden(index.row()) || isColumnHidden(index.column())
            )
        return;

    // Adjust vertical position

    if (hint == EnsureVisible)
    {
        hint = PositionAtCenter;
    }

    int viewportHeight = viewport()->height();
    int verticalOffset = verticalHeader()->offset();
    int verticalPosition = verticalHeader()->sectionPosition(index.row());
    int verticalIndex = verticalHeader()->visualIndex(index.row());
    int cellHeight = verticalHeader()->sectionSize(index.row());

    if (verticalPosition - verticalOffset < 0 || cellHeight > viewportHeight) {
        if (hint == EnsureVisible)
            hint = PositionAtTop;
    } else if (verticalPosition - verticalOffset + cellHeight > viewportHeight) {
        if (hint == EnsureVisible)
            hint = PositionAtBottom;
    }

    if (verticalScrollMode() == QAbstractItemView::ScrollPerItem) {

        if (hint == PositionAtBottom || hint == PositionAtCenter) {
            int h = (hint == PositionAtCenter ? viewportHeight / 2 : viewportHeight);
            int y = cellHeight;
            while (verticalIndex > 0) {
                int row = verticalHeader()->logicalIndex(verticalIndex - 1);
                y += verticalHeader()->sectionSize(row);
                if (y > h)
                    break;
                --verticalIndex;
            }
        }

        if (hint == PositionAtBottom || hint == PositionAtCenter || hint == PositionAtTop) {
            int hiddenSections = 0;
            if (verticalHeader()->sectionsHidden()) {
                for (int s = verticalIndex - 1; s >= 0; --s) {
                    int row = verticalHeader()->logicalIndex(s);
                    if (verticalHeader()->isSectionHidden(row))
                        ++hiddenSections;
                }
            }
            verticalScrollBar()->setValue(verticalIndex - hiddenSections);
        }

    } else { // ScrollPerPixel
        if (hint == PositionAtTop) {
            verticalScrollBar()->setValue(verticalPosition);
        } else if (hint == PositionAtBottom) {
            verticalScrollBar()->setValue(verticalPosition - viewportHeight + cellHeight);
        } else if (hint == PositionAtCenter) {
            verticalScrollBar()->setValue(verticalPosition - ((viewportHeight - cellHeight) / 2));
        }
    }

    update(index);
}
const QModelIndex MinosTableWidget::getFirstSelected() const
{
    QModelIndexList mi = selectionModel() ->selectedRows();

    if (mi.size())
        return mi[0];
    return QModelIndex();
}

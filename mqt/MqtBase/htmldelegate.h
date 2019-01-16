/*====================================================================================
    This file is part of AdjQt, the QT based version of the RSGB
    contest adjudication software.
    
    AdjQt and its predecessor AdjSQL are Copyright 1992 - 2016 Mike Goodey G0GJV 
 
    AdjQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AdjQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AdjQt in file gpl.txt.  If not, see <http://www.gnu.org/licenses/>.
    
======================================================================================*/

#ifndef HTMLDELEGATE_H
#define HTMLDELEGATE_H

#include "base_pch.h"

class HtmlDelegate : public QStyledItemDelegate
{
    Q_OBJECT
        qreal wmult = 1.0;
        qreal hmult = 1.0;
    public:
        HtmlDelegate(qreal wmult, qreal hmult);
        virtual ~HtmlDelegate() override;
        QSize docSize(QString text) const;
    protected:
        void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
        QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    private slots:
        void onListCompressionChanged(qreal h);
};
class TestDelegate : public HtmlDelegate
{
    Q_OBJECT

    public:
        TestDelegate(qreal wmult, qreal hmult):HtmlDelegate(wmult, hmult){}
    protected:
        void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
        QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
};

#endif // HTMLDELEGATE_H

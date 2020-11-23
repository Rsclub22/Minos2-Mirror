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

#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "htmldelegate.h"


HtmlDelegate::HtmlDelegate(qreal wmult, qreal hmult):wmult(wmult), hmult(hmult)
{
    connect(&MinosLoggerEvents::mle, SIGNAL(listCompressionChanged(qreal)),
            this, SLOT(onListCompressionChanged(qreal)));

}
HtmlDelegate:: ~HtmlDelegate()
{}

void HtmlDelegate::onListCompressionChanged(qreal h)
{
    hmult = h;
}
void HtmlDelegate::paint( QPainter *painter, const QStyleOptionViewItem &poption, const QModelIndex &index ) const
{
    QStyleOptionViewItem option = poption; // kill const

    initStyleOption( &option, index );

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml( option.text );

    // Painting item without text - gives highlighting etc
    option.text = QString();
    style->drawControl( QStyle::CE_ItemViewItem, &option, painter );

    QAbstractTextDocumentLayout::PaintContext ctx;

    QRect textRect = style->subElementRect( QStyle::SE_ItemViewItemText, &option );

    QRect oldRect = textRect;
    int oldHeight = textRect.bottom() - textRect.top();
    int newHeight = static_cast<int>(oldHeight*hmult);
    textRect.setBottom(textRect.top() + newHeight);
    textRect.setTop(textRect.top() + (newHeight - oldHeight)*2/3);

    painter->save();
    painter->translate( textRect.topLeft() );
    painter->setClipRect( oldRect.translated( -oldRect.topLeft() ) );
    doc.documentLayout() ->draw( painter, ctx );
    painter->restore();
}

QSize HtmlDelegate::sizeHint( const QStyleOptionViewItem &poption, const QModelIndex &index ) const
{
    QStyleOptionViewItem option = poption; // kill const

    initStyleOption( &option, index );

    return docSize(option.text);
}

QSize HtmlDelegate::docSize(QString text) const
{
    QTextDocument doc;
    doc.setHtml( text );
    doc.setTextWidth( -1 );
    return QSize( static_cast<int>(doc.idealWidth() * wmult), static_cast<int>(doc.size().height() * hmult) );
}


void TestDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    HtmlDelegate::paint(painter, option, index);
}

QSize TestDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize s = HtmlDelegate::sizeHint(option, index);
    QString text = option.text;
    return s;
}
QSize TestDelegate::docSize(QString text) const
{
    return HtmlDelegate::docSize(text);
}

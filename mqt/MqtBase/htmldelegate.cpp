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

#include "htmldelegate.h"

void HtmlDelegate::paint( QPainter *painter, const QStyleOptionViewItem &poption, const QModelIndex &index ) const
{
    QStyleOptionViewItem option = poption; // kill const

    initStyleOption( &option, index );

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml( option.text );

    /// Painting item without text
    option.text = QString();
    style->drawControl( QStyle::CE_ItemViewItem, &option, painter );

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    // But this gives crap colours

    //    if (option.state & QStyle::State_Selected)
    //        ctx.palette.setColor(QPalette::Text, option.palette.color(QPalette::Active, QPalette::HighlightedText));

    QRect textRect = style->subElementRect( QStyle::SE_ItemViewItemText, &option );
//    textRect.setBottom(textRect.top() + (textRect.bottom() - textRect.top()) / hmult);

    QRect oldRect = textRect;
    int oldHeight = textRect.bottom() - textRect.top();
    int newHeight = static_cast<int>(oldHeight*hmult);
    textRect.setBottom(textRect.top() + newHeight);

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

    QTextDocument doc;
    doc.setHtml( option.text );
    doc.setTextWidth( option.rect.width() );
    return QSize( static_cast<int>(doc.idealWidth() * wmult), static_cast<int>(doc.size().height() * hmult) );
}

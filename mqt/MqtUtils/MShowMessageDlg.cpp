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

#include "mqtUtils_pch.h"
#include <QSpacerItem>
#include <QGridLayout>
#include <QMessageBox>

//---------------------------------------------------------------------------
void mShowMessage( const QString &mess, QWidget *Owner )
{
    QMessageBox msgBox(Owner);
    msgBox.setText( mess );

    if (Owner)
    {
        QSpacerItem* horizontalSpacer = new QSpacerItem(Owner->width()*2/3, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = dynamic_cast<QGridLayout*>(msgBox.layout());
        if (layout)
            layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    }
    msgBox.exec();
}

void mShowMessage( const char *mess, QWidget *Owner )
{
    mShowMessage( QString(mess), Owner );
}

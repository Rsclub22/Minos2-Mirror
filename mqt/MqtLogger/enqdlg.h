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

#ifndef enqdlgH
#define enqdlgH 
#include "base_pch.h"
//---------------------------------------------------------------------------
extern bool enquireDialog (QWidget *owner, const QString &prompt, QString &Value );
extern bool enquireDialog (QWidget *owner, const QString &prompt, int &Value , int minval = -2147483647, int maxval = 2147483647);
extern bool enquireDialog (QWidget *owner, const QString &prompt, QString &Value, const QStringList sl, bool edittable);
//---------------------------------------------------------------------------
#endif

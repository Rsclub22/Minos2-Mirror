/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BASE_PCH_H
#define BASE_PCH_H
#include "XMPP_pch.h"

#include <QApplication>
#include <QMainWindow>

#include <QDialog>
#include <QTimer>

#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QStyle>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include <QKeyEvent>
#include <QTableView>
#include <QTableWidgetItem>
#include <QTreeView>
#include <QToolButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

#include <QDebug>
template <class itemtype >inline bool operator < (const MapWrapper<itemtype> &key1, const MapWrapper<itemtype> &key2)
{
    itemtype *i1 = key1.wt.data();
    itemtype *i2 = key2.wt.data();
    return *i1 < *i2;
}
template <class itemtype >inline bool operator == (const MapWrapper<itemtype> &key1, const MapWrapper<itemtype> &key2)
{
    return key1.wt.data() == key2.wt.data();
}

#include "minossplitter.h"

#include <math.h>
#include <time.h>

#include "fileutils.h"
#include "MinosParameters.h"
#include "ProfileEnums.h"
#include "mwin.h"
#include "CacheSelection.h"
#include "cutils.h"
#include "latlong.h"
#include "calcs.h"

#include "mults.h"
#include "contest.h"
#include "list.h"
#include "contacts.h"
#include "ScreenContact.h"
#include "MatchContact.h"
#include "ListContact.h"
#include "MatchCollection.h"
#include "validators.h"

#include "rigcontrolcommonconstants.h"
#include "MinosTestImport.h"

#include "MShowMessageDlg.h"
#include "MMessageDialog.h"

#include "TreeUtils.h"
#include "DisplayContestContact.h"

#include "BandList.h"
#include "PubSubValue.h"
#include "TinyUtils.h"

#include "waitcursor.h"

#endif

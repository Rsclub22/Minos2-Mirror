#ifndef TBUNDLEFRAME_H
#define TBUNDLEFRAME_H

#include "minositem.h"
#include <QFrame>

namespace Ui {
class TBundleFrame;
}

class SettingsBundle;
class ContestDetails;

class TBundleFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TBundleFrame(QWidget *parent = nullptr);
    ~TBundleFrame();

private:
    Ui::TBundleFrame *ui;

   ContestDetails *cd;
   bool suppressChange;
   SettingsBundle *bundle;
   MinosStringItem<QString> *bname;

public:  		// User declarations
   void initialise();
   void initialise( ContestDetails *cd, const QString &cap, SettingsBundle *bundle, MinosStringItem<QString> *name );
   bool doEdit( );
   void enableBundle( bool );

   void setBundleName(QString bn);
   QString getBundleName() const;
   private slots:
   void on_BundleEdit_clicked();
   void on_BundleSection_currentIndexChanged(int arg1);
};

#endif // TBUNDLEFRAME_H

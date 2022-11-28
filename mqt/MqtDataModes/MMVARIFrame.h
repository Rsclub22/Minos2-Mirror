#ifndef MMVARIFRAME_H
#define MMVARIFRAME_H

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>


#if !defined (_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
// as we don't want to change mmvarilib.h...
#include "mmvarilib.h"
#if !defined (_MSC_VER)
#pragma GCC diagnostic pop
#endif


namespace Ui {
class MMVARIFrame;
}

class MMVARIFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MMVARIFrame(QWidget *parent, QVBoxLayout *cwl, QLineEdit *sendEdit, QString fname, int inId, int outId);
    ~MMVARIFrame();

    void sendCharacters(const QString &);

private:
    Ui::MMVARIFrame *ui;

    QString fname;
    int mmvariWnd;
    QMap<QAction *, const char *> actionList;
    QMap<QMenu *, const char *> menuList;

    QLineEdit *sendEdit;

    MMVARILib::MMVARI *mmvari = nullptr;
    MMVARILib::XMMVView *mmview = nullptr;
    MMVARILib::XMMVView *mmview2 = nullptr;
    MMVARILib::XMMVLvl *mmlevel = nullptr;

    QVBoxLayout *mmvariVb = nullptr;

    // TX, RX, AFC, NET, ALIGN, mode combo, speed combo, TX Carrier, RX Carrier, db indicator

    QPushButton *txButton = nullptr;
    QPushButton *rxButton = nullptr;
    QPushButton *afcButton = nullptr;
    QPushButton *netButton = nullptr;
    QPushButton *alignButton = nullptr;

    QComboBox *modeCombo = nullptr;
    QComboBox *speedCombo = nullptr;

    QLabel *rxCarrier;
    QLabel *txCarrier;
    QLabel *snLabel;

    QAction *newAction(int n, QMenu *m, void (MMVARIFrame::*slotparam)());
    QAction *newAction(const char *text, QMenu *m, void (MMVARIFrame::*slotparam)() );
    QMenu *newMenu(QMenu *m, const char *text);
    QAction *newCheckableAction(const char *text, QMenu *m, void (MMVARIFrame::*slotparam)(bool) );
    QAction *newCheckableAction(const QString text, QMenu *m, void (MMVARIFrame::*slotparam)(bool) );

private slots:
    void onATC(bool checked);
    void onModeComboChanged(const QString &m);
    void onSpeedComboChanged(const QString &s);
    void txButtonClicked(bool checked);
    void rxButtonClicked(bool checked);
    void afcButtonClicked(bool checked);
    void netButtonClicked(bool checked);
    void alignButtonClicked(bool checked);

    void onHandleMMVARICOMException(int code, QString source, QString desc, QString help);
    void onMMVARISignal(const QString &,int,void *);
    void onMMVARIPropertyChanged(const QString&);

    void onHandleXMMVIEWCOMException(int code, QString source, QString desc, QString help);
    void onXMMVIEWSignal(const QString &,int,void *);
    void onXMMVIEWPropertyChanged(const QString&);

    void OnMode(int, int, QString);
    void OnDrawFFT(int, int &);
    void OnTxCarrier(int);
    void OnRxCarrier(int, int);
    void OnSpeed(int, double);
    void OnRxChar(int,QString,int);
    void OnTiming(int,int,int);
    void OnGetTxChar(int&);
    void OnDrawWave(int, int&, int&);
    void OnPTT(int);
    void OnNET(int);
    void OnTxState(int);
    void OnPlayBack(int);
    void OnClockAdjust(int &);
    void OnError(int);


};

#endif // MMVARIFRAME_H

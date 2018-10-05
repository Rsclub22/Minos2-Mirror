#ifndef SERVERDETAILS_H
#define SERVERDETAILS_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>


namespace Ui {
class ServerDetails;
}
class ServerDetails : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDetails(QWidget *parent = nullptr);
    virtual ~ServerDetails() override;

private slots:
    void on_closeButton_clicked();

    void on_refreshButton_clicked();

private:
    Ui::ServerDetails *ui;
    
    void refresh();
protected:
    virtual void showEvent(QShowEvent *) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

};

#endif // SERVERDETAILS_H

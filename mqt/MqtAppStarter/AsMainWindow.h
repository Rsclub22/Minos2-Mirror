#ifndef ASMAINWINDOW_H
#define ASMAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QAction>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void startTimer_Timeout();

    void on_appsButton_clicked();
    void on_startAppsButton_clicked();
    void on_appSelectButton_clicked();

    void on_closeButton_clicked();

    void on_stdOutLine(QString);

    void ExitActionExecute();
    void FontEditAcceptActionExecute();
    void LanguageAcceptActionExecute();
    void ManageHamlibActionExecute();

private:
    Ui::MainWindow *ui;
    QLabel *sbLabel;
    QTimer startTimer;
    QMetaObject::Connection m_connection;

    QMenu *languagesMenu;
    QAction *FontEditAcceptAction;
    QAction *LanguageAcceptAction;
    QAction *ManageHamlibAction;
    QAction *ExitAction;
    QAction *lastLanguageSelected = nullptr;

    QMap<QAction *, const char *> actionList;
    QMap<QMenu *, const char *> menuList;

    bool started = false;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void start();
    QAction *newAction(const char *text, QMenu *m, void (MainWindow::*slotparam)());
    QMenu *newMenu(QMenu *m, const char *text);
};

#endif // ASMAINWINDOW_H

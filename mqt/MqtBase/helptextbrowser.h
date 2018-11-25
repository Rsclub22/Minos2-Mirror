#ifndef HELPTEXTBROWSER_H
#define HELPTEXTBROWSER_H

#include <QTextBrowser>
#include <QHelpEngine>

class HelpTextBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    HelpTextBrowser(QWidget* parent);
    HelpTextBrowser(QHelpEngine *engine, QWidget* parent);
    virtual QVariant loadResource (int type, const QUrl& url) override;
    void setHelpEngine(QHelpEngine *engine);
private:
    QHelpEngine* helpEngine;
public slots:
    void on_anchorClicked(const QUrl &url);
};

#endif // HELPTEXTBROWSER_H

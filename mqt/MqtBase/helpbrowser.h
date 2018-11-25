#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include "base_pch.h"
#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>

namespace Ui {
class HelpBrowser;
}

class HelpBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit HelpBrowser(QString collectionFile, QUrl startUrl, QWidget *parent = nullptr);

    void setUrl(QUrl url);

    ~HelpBrowser() override;

private:
    Ui::HelpBrowser *ui;
    QHelpEngine *helpEngine;
    QHelpContentWidget *content;
    QHelpIndexWidget *index;

    void doCloseEvent();


public slots:
    void on_indexFilter_textChanged(QString text);
    void on_splitter_splitterMoved(int pos, int index);
    void on_linksActivated(const QMap<QString, QUrl> &map, const QString &keyword);

    virtual void reject() override;
    virtual void accept() override;
};

#endif // HELPBROWSER_H

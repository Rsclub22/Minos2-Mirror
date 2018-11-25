#include "base_pch.h"

#include <QHelpEngine>

#include "helpbrowser.h"
#include "ui_helpbrowser.h"

HelpBrowser::HelpBrowser(QString collectionFile, QUrl startUrl, QWidget *parent):
    QDialog(parent),
    ui(new Ui::HelpBrowser)

{
    ui->setupUi(this);

    // Restore any settings
    QSettings settings;
    restoreGeometry(settings.value("HelpBrowser/geometry").toByteArray());
    ui->splitter->restoreState(settings.value("HelpBrowser/splitterSizes").toByteArray());

    // Create the Help Engine
    helpEngine = new QHelpEngine(QString(collectionFile));
    helpEngine->setupData();

    // Add the contentWidget to the ui
    content = helpEngine->contentWidget();
    ui->contents_layout->addWidget(content);

    // Add the indexWidget to the ui
    index = helpEngine->indexWidget();
    ui->index_layout->addWidget(index);

    // set the helpEngine to the textBrower and set the startUrl
    ui->textBrowser->setHelpEngine(helpEngine);
    ui->textBrowser->setSource(startUrl);

    // Connect the helpEngine Signals
    QObject::connect(helpEngine->contentWidget(), SIGNAL(linkActivated(QUrl)), ui->textBrowser, SLOT(setSource(QUrl)));
    QObject::connect(helpEngine->indexWidget(), SIGNAL(linkActivated(QUrl, QString)), ui->textBrowser, SLOT(setSource(QUrl)));
    QObject::connect(helpEngine->indexWidget(), SIGNAL(linksActivated(const QMap<QString, QUrl>, QString)),
                     this, SLOT(on_linksActivated(const QMap<QString, QUrl>, QString)));
}
void HelpBrowser::doCloseEvent()
{
    QSettings settings;
    settings.setValue("HelpBrowser/geometry", saveGeometry());
}
void HelpBrowser::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void HelpBrowser::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void HelpBrowser::setUrl(QUrl url)
    //  Use to programatically set the textBrowser source
{
    ui->textBrowser->setSource(url);
}

void HelpBrowser::on_indexFilter_textChanged(QString text)
{
    helpEngine->indexModel()->filter(text, "");
}

void HelpBrowser::on_linksActivated(const QMap<QString, QUrl> &map, const QString &keyword)
    //  Slot triggered when the text in the index window
    //  is double clicked and the keyword refers to more than
    //  one topic.

    //  map: The topics and their QUrls

    //  Shows a QInputDialog and displays the topic selected
    //  from the dialogue combo box.

    //  The HTML files must have unique <title> tags
    //  to indicate the topic."""
{
    QString topic;
    bool ok;

    topic = QInputDialog::getItem(this, tr("Choose"), tr("Select topic for: ") + keyword, map.keys(), 0, false, &ok);

    if (ok)
        ui->textBrowser->setSource(map[topic]);
}

void HelpBrowser::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    settings.setValue("HelpBrowser/splitterSizes", ui->splitter->saveState());
}

HelpBrowser::~HelpBrowser()
{
    delete ui;
}

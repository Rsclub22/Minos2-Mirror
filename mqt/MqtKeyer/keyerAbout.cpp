#include <QDesktopServices>
#include <QUrl>

#include "keyerAbout.h"
#include "ui_keyerAbout.h"

keyerAbout::keyerAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyerAbout)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->AboutMemo->setText(QString("<h1>") + tr("Welcome to Minos Voice Keyer Version ") + STRINGVERSION + " " + PRERELEASETYPE +
                           " Beta" + "</h1><br><a href=\"http://minos.sourceforge.net/\">http://minos.sourceforge.net</a>");
    ui->AboutMemo->setTextFormat(Qt::RichText);
    ui->AboutMemo->setTextInteractionFlags(Qt::TextBrowserInteraction);

}

keyerAbout::~keyerAbout()
{
    delete ui;
}

void keyerAbout::on_pushButton_clicked()
{
    close();
}

void keyerAbout::on_AboutMemo_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

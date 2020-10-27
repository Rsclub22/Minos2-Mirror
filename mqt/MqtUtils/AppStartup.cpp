#include "mqtUtils_pch.h"
#include <QPalette>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QTranslator>

static bool appClosing = false;
static QString appStartupName;
static QString executableName;
static QString currentLanguage;

static QSharedPointer<QTranslator> translator;
static QSharedPointer<QTranslator> qtTranslator;

QString getAppStartupName()
{
    return appStartupName;
}
static QtMessageHandler oldHandler = nullptr;
void myMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg)
{
    // catch (and trace) application output before a crash
    oldHandler(type, context, msg);

    if (appClosing)
        return;

    QString mtype;
    switch (type)
    {
    case QtDebugMsg:
        mtype = "Debug";
        break;
#if QT_VERSION > QT_VERSION_CHECK(5, 4, 0)
    case QtInfoMsg:
        mtype = "Info";
        break;
#endif
    case QtWarningMsg:
        mtype = "Warning";
        break;
    case QtCriticalMsg:
        mtype = "Critical";
        break;
    case QtFatalMsg:
        mtype = "Fatal";
        break;
    }
    QString res = QString("AppMessageHandler (%1, %2 %3:%4): %5").arg(mtype).arg(context.file).arg(context.line).arg(context.function).arg(msg);
    trace(res);
    if (type == QtCriticalMsg || type == QtFatalMsg)
    {
        QMessageBox msgBox(nullptr);
        msgBox.setText( res );

        msgBox.exec();
    }
}

QVector<Translation> getLanguages()
{
    QVector<Translation> locs;
    QString searchString = GetCurrentDir() + "/Bin/translations";

    QDirIterator files( searchString, QDir::Files | QDir::NoSymLinks , QDirIterator::NoIteratorFlags );
    while ( files.hasNext() )
    {
        files.next();
        QFileInfo finfo = files.fileName();

        QString fileExt = finfo.suffix();

        if ( fileExt.compare("qm") == 0 )
        {
            QString fname =finfo.baseName();
            if (fname.startsWith(executableName))
            {
                int uscore = fname.indexOf('_');
                QString l = fname.mid(uscore + 1);
                QLocale loc(l);
                QString lname = loc.nativeLanguageName();
                Translation t;
                t.code = l;
                t.dispName = lname;

                locs.push_back(t);
            }
        }
    }
//    locs.sort();
    return locs;
}

void switchTranslation(QString loc)
{
    QApplication *qa = dynamic_cast<QApplication *>(QApplication::instance());

    QSharedPointer<QTranslator> myappTranslator(new QTranslator());    // which goes out of scope :(
    QSharedPointer<QTranslator> myqtTranslator(new QTranslator());    // which goes out of scope :(

    QString qtlocfile = QString("Bin/translations/") + "qtbase_" + loc;
    bool qtloadOK = myqtTranslator->load(qtlocfile);
    bool qtinstallOK = qa->installTranslator(myqtTranslator.data());

    if (!qtloadOK || qtinstallOK)
    {
        qtlocfile = QString("Bin/translations/") + "qt_" + loc;
        qtloadOK = myqtTranslator->load(qtlocfile);
        qtinstallOK = qa->installTranslator(myqtTranslator.data());
    }

    QString locfile = "Bin/translations/" + executableName + "_" + loc;
    bool loadOK = myappTranslator->load(locfile);
    bool installOK = qa->installTranslator(myappTranslator.data());

    if (translator)
    {
        qa->removeTranslator(translator.data());
    }
    if (qtTranslator)
    {
        qa->removeTranslator(qtTranslator.data());
    }

    qtTranslator = myqtTranslator;
    translator = myappTranslator;
    trace(QString("Translation file %1 loaded:%2 installed:%3").arg(qtlocfile).arg(qtloadOK).arg(qtinstallOK));
    trace(QString("Translation file %1 loaded:%2 installed:%3").arg(locfile).arg(loadOK).arg(installOK));

    QSettings settings;
    currentLanguage = loc;
    if (loc == QLocale::system().name())
    {
        settings.remove("language");
    }
    else
    {
        settings.setValue( "language", loc );
    }
}
static QString getAppLanguage()
{
    QSettings settings;
    QVariant qlang = settings.value( "language" );
    if ( qlang == QVariant() )
    {
        qlang = QLocale::system().name();
    }
    return qlang.toString();
}
void setAppLanguage(QString loc)
{
    if (loc.startsWith("LANG "))
    {
        loc = loc.remove(0, 5);
    }
    switchTranslation(loc);
}

QString getCurrentLanguage()
{
    return currentLanguage;
}

void appStartup(const QString &pappName)
{
    oldHandler = qInstallMessageHandler(myMessageOutput);
    executableName = QCoreApplication::instance()->applicationName();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appStartupName = env.value("MQTRPCNAME", "") ;

    if (appStartupName.isEmpty())
    {
        appStartupName = pappName;
    }

    QApplication::setOrganizationName( "Minos2Qt" );
    QApplication::setOrganizationDomain( "g0gjv.org.uk" );
    QApplication::QCoreApplication::setApplicationName( appStartupName );

    QApplication *qa = dynamic_cast<QApplication *>(QApplication::instance());

    QCommandLineParser parser;

    QString languageName = getAppLanguage();
    QCommandLineOption languageOption({"l", "lang"}, "language", "languageName", "");
    parser.addOption(languageOption);

    parser.parse(QCoreApplication::instance()->arguments());

    if (parser.isSet(languageOption))
    {
        languageName = parser.value(languageOption);
    }

    qa->setStyleSheet(QString("[readOnly=\"true\"] { background-color: %0 }").arg(qa->palette().color(QPalette::Window).name(QColor::HexRgb)));

    if (!DirectoryExists("./Configuration"))
    {
#ifdef Q_OS_ANDROID
        bool createOK = CreateDir("./Configuration");
        if (!mShowOKCancelMessage(0, createOK?"./Config created":"create ./Config failed; Cancel for abort"))
        {
            exit(0);
        }
#else
        // try for executable directory
        QString fpath = QCoreApplication::applicationDirPath();

        if (DirectoryExists(fpath + "/../Configuration"))
        {
            QDir::setCurrent(fpath + "/..");
        }
        int confTries = 0;
        while (!DirectoryExists("./Configuration") )
        {
            if (++confTries > 2)
            {
                exit(-1);
            }
            QString destDir = QFileDialog::getExistingDirectory(
                        nullptr,
                        "Set Minos Working Directory",      // we are pre-translation here...
                        fpath,
                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                        );
            if ( !destDir.isEmpty() )
            {

                if (destDir.toUpper().indexOf("/CONFIGURATION") == destDir.size() - QString("/Configuration").size())
                {
                    destDir = destDir.left(destDir.size() - QString("/Configuration").size());
                }
                QDir::setCurrent(destDir);
            }
        }
#endif
    }
    enableTrace( "./TraceLog", appStartupName + "_" );

    QString Version = QString(STRINGVERSION)  + " " + PRERELEASETYPE ;

    QString title = appStartupName;

    QString compiler;
#ifndef NDEBUG
    compiler += " DEBUG ";
#endif

#define STRING(s) #s
#ifdef __GNUC__

        compiler += QString(" GCC");
#endif
#ifdef _MSC_FULL_VER
        compiler += QString(" MSVC");
#endif
#ifdef Q_PROCESSOR_X86_32
        title += " 32 bit X86 build Version ";
#else
#ifdef Q_PROCESSOR_X86_64
        title += " 64 bit X86 build Version " ;
#else
        title += " Other build Version ";
#endif
#endif
    title += Version + compiler;

    trace(title);

    register_frequency_types();

    setAppLanguage(languageName);
}


void setAppFont()
{
    QSettings settings;
    QVariant qfont = settings.value( "font" );
    if ( qfont != QVariant() )
    {
        QApplication::setFont( qfont.value<QFont>() );
    }
}
void setAppFont(QString fs)
{
    if (fs.startsWith("Font "))
    {
        fs = fs.remove(0, 5);
    }
    QFont f;
    if (f.fromString(fs))
    {
        trace(QString("Setting font to %1").arg(fs));
        QApplication::setFont( f );
#ifndef Q_OS_WIN
// as timings are different under linux
        for ( auto const &widget: QApplication::allWidgets() )
        {
            widget->setFont(f);
            widget->update();
        }
#endif
    }
    else
    {
        trace(QString("Failed Setting font to %1").arg(fs));
    }
}

void setAppClosing()
{
    appClosing = true;
}

void executeStdIn(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);
    if (cmd.indexOf("Font ", 0, Qt::CaseInsensitive) >= 0)
    {
        setAppFont(cmd);
    }
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        QApplication::closeAllWindows();
    }
}


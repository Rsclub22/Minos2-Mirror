#include <QPalette>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>
#include <QStandardPaths>

#include "fileutils.h"
#include "MTrace.h"
#include "SecondInstall.h"

#include "AppStartup.h"
#include "frequency.h"
#include "qdiriterator.h"

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
        QFileInfo finfo(files.fileName());

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
        loc.remove(0, 5);
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

    QApplication::setOrganizationName( SecondInstall::getOrgName() );
    QApplication::setOrganizationDomain( "g0gjv.org.uk" );
    QApplication::QCoreApplication::setApplicationName( appStartupName );

    QApplication *qa = dynamic_cast<QApplication *>(QApplication::instance());

    qa->setStyleSheet(QString("[readOnly=\"true\"] { background-color: %0 }").arg(qa->palette().color(QPalette::Window).name(QColor::HexRgb)));

// For Mac Users, set the default directory to be Documents/Minos

#ifdef Q_OS_MACOS
    QString sharedPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation,"",QStandardPaths::LocateDirectory);
    QDir::setCurrent(sharedPath);
    if (!DirectoryExists("./Minos2")) {
        QDir().mkdir("Minos2");
    }
    QDir::setCurrent("./Minos2");
#elif defined(Q_OS_IOS)
    QString sharedPath = sharedDirectory("group.minos2").toLocalFile();
#endif
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

#ifndef Q_OS_MACOS

        if (DirectoryExists(fpath + "/../Configuration"))
        {
            QDir::setCurrent(fpath + "/..");
        }
#else
        // We have no configuration directory so copy default one
        // Also Create a logs directory and copy Docs
        cpDir(QString(fpath+"/../Resources/Configuration"), QString("./Configuration"));

        if (!DirectoryExists("Logs")) {
            QDir().mkdir("Logs");
        }
        if (!DirectoryExists("Help")) {
            cpDir(QString(fpath+"/../Resources/Help"), QString("Help"));
        }
        if (!DirectoryExists("./Docs")) {
            cpDir(QString(fpath+"/../Resources/Docs"), QString("Docs"));
        }

#endif
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

    QString Version = QString(STRINGVERSION)  + " " + PRERELEASETYPE + " " + SecondInstall::getSecondInstallText() ;

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

    QCommandLineParser parser;

    QString languageName = getAppLanguage();
    QCommandLineOption languageOption({"l", "lang"}, "language", "languageName", "");
    parser.addOption(languageOption);

    QStringList args = QCoreApplication::instance()->arguments();
    trace("Arguments " + args.join("|"));

    parser.process(args);

    if (parser.isSet(languageOption))
    {
        languageName = parser.value(languageOption);
    }
    else
    {
        trace("LanguageOption not found");
    }

    trace(QString("Language Name %1").arg(languageName));

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
        fs.remove(0, 5);
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

bool cpDir(const QString &srcPath, const QString &dstPath)
{
    QDir parentDstDir(QFileInfo(dstPath).path());
    if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
        return false;

    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir()) {
            if (!cpDir(srcItemPath, dstItemPath)) {
                return false;
            }
        } else if (info.isFile()) {
            if (!QFile::copy(srcItemPath, dstItemPath)) {
                return false;
            }
        } else {
            qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
        }
    }
    return true;
}


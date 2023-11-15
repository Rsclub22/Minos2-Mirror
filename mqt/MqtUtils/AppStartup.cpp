#include <QDebug>
#include <QPalette>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

#include "regsettings.h"
#include "fileutils.h"
#include "MTrace.h"
#include "SecondInstall.h"

#include "AppStartup.h"
#include "frequency.h"
#include "qdiriterator.h"

static bool appClosing = false;
static QString appStartupName;
static QString executableName;
static QString executablePath;
static QString currentLanguage;

static QSharedPointer<QTranslator> translator;
static QSharedPointer<QTranslator> qtTranslator;

QString getAppExecutable()
{
    return executablePath;
}
QString getAppExecutableName()
{
    return executableName;
}
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
    QString searchString = getDirectoryLocation(dlTranslations);

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

    QString qtlocfile = getDirectoryLocation(dlTranslations) + "/qtbase_" + loc;
    bool qtloadOK = myqtTranslator->load(qtlocfile);
    bool qtinstallOK = qa->installTranslator(myqtTranslator.data());

    if (!qtloadOK || qtinstallOK)
    {
        qtlocfile = getDirectoryLocation(dlTranslations) + "/qt_" + loc;
        qtloadOK = myqtTranslator->load(qtlocfile);
        qtinstallOK = qa->installTranslator(myqtTranslator.data());
    }

    QString locfile = getDirectoryLocation(dlTranslations) + "/" + executableName + "_" + loc;
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

    RegSettings settings;
    currentLanguage = loc;
    if (loc == QLocale::system().name())
    {
        settings.getSettings().remove("language");
    }
    else
    {
        settings.getSettings().setValue( "language", loc );
    }
}
static QString getAppLanguage()
{
    RegSettings settings;
    QVariant qlang = settings.getSettings().value( "language" );
    if ( qlang == QVariant() )
    {
        QSettings s;
        qlang = s.value( "language" );
        if ( qlang == QVariant() )
        {
            qlang = QLocale::system().name();
        }
        else
        {
            settings.getSettings().setValue("language", qlang.toString());
        }
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
    // This gets reset later, but at this point it is the launch executable name
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

    QString fpath = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MACOS
    QString sharedPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation,"",QStandardPaths::LocateDirectory);
    QDir::setCurrent(sharedPath);
    if (!DirectoryExists("./Minos2")) {
        QDir().mkdir("Minos2");
    }
    QDir::setCurrent("./Minos2");

    // We have no configuration directory so copy default one
    // Also Create a logs directory and copy Docs

    // cpDir does copy if newer, so a new installation updates the old one

    if (!DirectoryExists("Configuration")) {
        QDir().mkdir("Configuration");
    }
    if (!DirectoryExists("Logs")) {
        QDir().mkdir("Logs");
    }
    if (!DirectoryExists("Lists")) {
        QDir().mkdir("Lists");
    }
    if (!DirectoryExists("Docs")) {
        QDir().mkdir("Docs");
    }
    cpDir(QString(fpath+"/../Resources/Configuration"), QString("./Configuration"));
    cpDir(QString(fpath+"/../Resources/Docs"), QString("./Docs"));

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

#ifndef Q_OS_MACOS
        if (DirectoryExists(fpath + "/../Configuration"))
        {
            QDir::setCurrent(fpath + "/..");
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
    else if (fpath.contains("/build"))
    {
        // Don't do this from an issue system
        // Configuration already exists; we should probably "copy if newer"

        QString srcMaster = fpath;
        while(DirectoryExists(srcMaster))
        {
            QFileInfo src(srcMaster + "/..");
            srcMaster = src.canonicalFilePath();
            if (DirectoryExists(srcMaster) && DirectoryExists(srcMaster + "/mqt/ControlFiles/Configuration"))
            {
                break;
            }
        }
        if (DirectoryExists(srcMaster) && DirectoryExists(srcMaster + "/mqt/ControlFiles/Configuration"))
        {
            // copyifnewer the master config
            QString destConfig = QDir::currentPath() + "/Configuration";


            cpDir(srcMaster + "/mqt/ControlFiles/Configuration", destConfig);
#if defined( Q_OS_MACOS)
            cpDir(destConfig + "/OSXFiles", destConfig);
#elif defined(Q_OS_WIN)
            cpDir(destConfig + "/WindowsFiles", destConfig);
#else
            cpDir(destConfig + "/LinuxFiles", destConfig);
#endif
            QDir osx(destConfig + "/OSXFiles");
            osx.removeRecursively();
            QDir win(destConfig + "/WindowsFiles");
            win.removeRecursively();
            QDir linuxf(destConfig + "/LinuxFiles");    // straight "linux" is objected to
            linuxf.removeRecursively();
        }
        if (DirectoryExists(srcMaster) && DirectoryExists(srcMaster + "/mqt/Docs"))
        {
            // copyifnewer the master config
            QString destDocs = QDir::currentPath() + "/Docs";

            cpDir(srcMaster + "/mqt/Docs", destDocs);
        }
    }
    enableTrace( getDirectoryLocation(dlTraceLog), appStartupName + "_" );

    trace(QSysInfo::prettyProductName());
    trace(QSysInfo::buildAbi());
    trace(qVersion());


    QString sysinfo = QString("%1 %2 %3").arg(appStartupName,
                                      QSysInfo::kernelType(),
                                      QSysInfo::buildCpuArchitecture()
                                      );
    trace(sysinfo);

    QString Version = QString(STRINGVERSION)  + " " + PRERELEASETYPE + " " + SecondInstall::getSecondInstallText() ;
    QString compiler;
#ifndef NDEBUG
    compiler += " DEBUG ";
#endif

#ifdef __GNUC__

        compiler += QString(" GCC");
#endif
#ifdef _MSC_FULL_VER
        compiler += QString(" MSVC");
#endif

    trace(appStartupName + " " + Version + " " + compiler);

    QCommandLineParser parser;

    QString languageName = getAppLanguage();
    QCommandLineOption languageOption({"l", "lang"}, "language", "languageName", "");
    parser.addOption(languageOption);

    QStringList args = QCoreApplication::instance()->arguments();
    trace("Arguments " + args.join("|"));

    parser.process(args);

    executablePath = args[0];

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
    RegSettings settings;
    QVariant qfont = settings.getSettings().value( "font" );
    if ( qfont == QVariant() )
    {
        QSettings s;
        qfont = s.value( "font" );
        settings.getSettings().setValue("font", qfont);
    }
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
    if (QFileInfo(srcPath).isDir())
    {
        QDir dstDir;
        if (!dstDir.mkpath(dstPath))
        {
            return false;
        }
    }
    else
    {
        QString parentDir(QFileInfo(dstPath).path());
        if (!DirectoryExists(parentDir))
        {
            QDir parentDstDir(QFileInfo(dstPath).path());
            if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
                return false;
        }
    }

    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir())
        {
            if (!cpDir(srcItemPath, dstItemPath))
            {
                return false;
            }
        }
        else if (info.isFile())
        {
            if (FileExists(dstItemPath))
            {
                // check if src is newer than dest
                QFileInfo srcInfo(srcItemPath);
                QFileInfo dstInfo(dstItemPath);
                if (dstInfo.fileTime(QFileDevice::FileModificationTime)
                    >= srcInfo.fileTime(QFileDevice::FileModificationTime))
                {
                    continue;
                }
                QFile::remove(dstItemPath);
            }

            if (!QFile::copy(srcItemPath, dstItemPath)) {
                return false;
            }
        } else {
            qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
        }
    }
    return true;
}


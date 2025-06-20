VERSION=2.7.999.0000
DEFINES += STRINGVERSION=\\\"$$VERSION\\\"
#DEFINES += PRERELEASETYPE=\\\"Pre-Beta\\\"
DEFINES += PRERELEASETYPE=\\\"Beta\\\"
#DEFINES += PRERELEASETYPE=\\\"RC1\\\"
#DEFINES += PRERELEASETYPE=\\\"\\\"

#DEFINES += SECONDINSTALL
# call using
# qmake.exe "SECONDINSTALL=true" ..\mqt\mqt.pro  -spec win32-g++
defined(SECONDINSTALL, var) {
    equals(SECONDINSTALL, "true"){
    DEFINES += SECONDINSTALL=SecondInstall
    }
}

CONFIG += c++17
DEFINES += TIXML_USE_STL
DEFINES += NOMINMAX

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
    QMAKE_TARGET_BUNDLE_PREFIX = uk.org.g0gjv.minos
    MY_ENTITLEMENTS.name = CODE_SIGN_ENTITLEMENTS
    MY_ENTITLEMENTS.value = ../ControlFiles/minos.entitlements
    QMAKE_MAC_XCODE_SETTINGS += MY_ENTITLEMENTS

#    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
    QMAKE_APPLE_DEVICE_ARCHS = x86_64
}

win32: {
versionAtLeast(QT_VERSION, 6.5.0){
INC_MAP = 1
DEFINES += INC_MAP
}
lessThan(QT_VERSION, 6.0.0) {
INC_MAP = 1
DEFINES += INC_MAP
}
}

mac: {
INC_MAP = 1
DEFINES += INC_MAP
CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-reorder -Wold-style-cast -DNDEBUG  -Winvalid-pch
CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-reorder -Wold-style-cast  -Winvalid-pch

} else {
# Map still not working on all Linux
#INC_MAP = 1
#DEFINES += INC_MAP
*g++*:CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-reorder -Wold-style-cast -DNDEBUG  -Winvalid-pch
else:*g++*:CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations -Wno-reorder -Wold-style-cast  -Winvalid-pch
}
DEFINES += _CRT_SECURE_NO_WARNINGS
DEFINES *= QT_USE_QSTRINGBUILDER
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES *= QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


CONFIG *= lrelease
TRANSLATIONS *= translations/minos_en_GB.ts \
                translations/minos_fr_FR.ts

# Others we may want eventually
#                translations/minos_da.ts \
#                translations/minos_de.ts \
#                translations/minos_it.ts \
#                translations/minos_nl.ts \
#                translations/minos_no.ts \
#                translations/minos_sv.ts \

HEADERS +=

SOURCES +=



VERSION=2.4.0.999
DEFINES += STRINGVERSION=\\\"$$VERSION\\\"
DEFINES += PRERELEASETYPE=\\\"Pre-Beta\\\"
#DEFINES += PRERELEASETYPE=\\\"RC2\\\"
#DEFINES += PRERELEASETYPE=\\\"\\\"
CONFIG += c++11
DEFINES += TIXML_USE_STL
*g++*:CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast -DNDEBUG  -Winvalid-pch
else:*g++*:CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast  -Winvalid-pch

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


CONFIG += lrelease
TRANSLATIONS += translations/minos_en_GB.ts \
                translations/minos_fr_FR.ts

# Others we may want eventually
#                translations/minos_da.ts \
#                translations/minos_de.ts \
#                translations/minos_it.ts \
#                translations/minos_nl.ts \
#                translations/minos_no.ts \
#                translations/minos_sv.ts \



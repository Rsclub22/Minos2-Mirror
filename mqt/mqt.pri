VERSION=2.4.0.1
DEFINES += STRINGVERSION=\\\"$$VERSION\\\"
#DEFINES += PRERELEASETYPE=\\\"Pre-Beta\\\"
#DEFINES += PRERELEASETYPE=\\\"Beta\\\"
#DEFINES += PRERELEASETYPE=\\\"RC2\\\"
DEFINES += PRERELEASETYPE=\\\"\\\"
CONFIG += c++11
DEFINES += TIXML_USE_STL
*g++*:CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast -DNDEBUG  -Winvalid-pch
else:*g++*:CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast  -Winvalid-pch

DEFINES += _CRT_SECURE_NO_WARNINGS
DEFINES *= QT_USE_QSTRINGBUILDER

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



VERSION=2.2.0.0
DEFINES += STRINGVERSION=\\\"$$VERSION\\\"
#DEFINES += PRERELEASETYPE=\\\"Pre-Beta\\\"
#DEFINES += PRERELEASETYPE=\\\"RC2\\\"
DEFINES += PRERELEASETYPE=\\\"\\\"
CONFIG += c++11
DEFINES += TIXML_USE_STL
*g++*:CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast -DNDEBUG  -Winvalid-pch
else:*g++*:CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast  -Winvalid-pch

DEFINES += _CRT_SECURE_NO_WARNINGS

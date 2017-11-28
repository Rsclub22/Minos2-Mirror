VERSION=0.8.1.0
DEFINES += STRINGVERSION=\\\"$$VERSION\\\"
DEFINES += PRERELEASETYPE=\\\"rc2\\\"
CONFIG += c++11
DEFINES += TIXML_USE_STL
*g++*:CONFIG(release, debug|release): QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast -DNDEBUG
else:*g++*:CONFIG(debug, debug|release):QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder -Wold-style-cast

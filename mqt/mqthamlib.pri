!macx: HAMLIBDIR = $$absolute_path(../../hamlib-w32-3.3)

macx: HAMLIBDIR = /usr/local

win32: INCLUDEPATH += $$HAMLIBDIR/include

win32-g++: LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-2
unix: LIBS += -lhamlib

macx: { 
	LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
	INCLUDEPATH += -I $${HAMLIBDIR}/include
} 

message('Include:  $${INCLUDEPATH}')
message('LIBS:  $${LIBS}')
macx:  message('BUILDING FOR MACOS')


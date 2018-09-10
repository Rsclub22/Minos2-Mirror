win32: {
   HAMLIBDIR = $$absolute_path(../../hamlib-w32-3.3)
   win32: INCLUDEPATH += $$HAMLIBDIR/include
   win32-g++: LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
   msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-2
}

unix: {
   LIBS += -lhamlib
}

macx: { 
   HAMLIBDIR = /usr/local
   LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
	INCLUDEPATH += -I $${HAMLIBDIR}/include
   message('Include:  $${INCLUDEPATH}')
   message('LIBS:  $${LIBS}')
   message('BUILDING FOR MACOS')
}



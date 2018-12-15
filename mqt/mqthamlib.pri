win32: {
   win32-g++* {
      contains(QT_ARCH, i386) {
      HAMLIBDIR = $$absolute_path(../../hamlib-w32-3.3)
      }
   contains(QT_ARCH, x86_64) {
      HAMLIBDIR = $$absolute_path(../../hamlib-w64-3.3)
      }
      LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
   } else {
      contains(QT_ARCH, i386) {
         HAMLIBDIR = $$absolute_path(../../hamlib-w32-3.3)
      }
      contains(QT_ARCH, x86_64) {
         HAMLIBDIR = $$absolute_path(../../hamlib-w64-3.3)
      }
      msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-2
}
   INCLUDEPATH += $$HAMLIBDIR/include

}

unix: {
   LIBS += -lhamlib
}

macx: { 
   HAMLIBDIR = /usr/local
   LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
   INCLUDEPATH += $${HAMLIBDIR}/include
}



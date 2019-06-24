# Allow testing of pre-release hamlib library
# comment out HAMLIBVER to use the current release
# The files need to be in C:\Qt_Projects
# Also make sure the Windows Path statement contains the path to the Hamlib Bin Dir
# For example C:\Qt_Projects\hamlib-w32-4.0\bin


CONFIG += hamlib_V4_0

CONFIG(hamlib_V4_0){

    win32: {
       win32-g++* {
          contains(QT_ARCH, i386) {
          HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.0)
          }
       contains(QT_ARCH, x86_64) {
          HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.0)
          }
          LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
       } else {
          contains(QT_ARCH, i386) {
             HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.0)
          }
          contains(QT_ARCH, x86_64) {
             HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.0)
          }
          msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-2
          msvc: DEFINES += DLL_EXPORT
    }
       INCLUDEPATH += $$HAMLIBDIR/include

    }
}else {

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
          msvc: DEFINES += DLL_EXPORT
    }
       INCLUDEPATH += $$HAMLIBDIR/include

    }
}


unix: {
   LIBS += -lhamlib
}

macx: { 
   HAMLIBDIR = /usr/local
   LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
   INCLUDEPATH += $${HAMLIBDIR}/include
}



# Make sure the Windows Path statement contains the path to the Hamlib Bin Dir
# For example C:\Qt_Projects\hamlib-w32-4.6\bin

HAMLIBVERSION=4.6
DEFINES += HAMLIBVERSION=\\\"$$HAMLIBVERSION\\\"
message(Using hamlib V$$HAMLIBVERSION)

win32: {
    win32-g++* {
          contains(QT_ARCH, i386) {
          HAMLIBDIR = $$absolute_path(../../hamlib-w32-$$HAMLIBVERSION)
          message(Hamlib path is $$HAMLIBDIR)
          }
          contains(QT_ARCH, x86_64) {
          HAMLIBDIR = $$absolute_path(../../hamlib-w64-$$HAMLIBVERSION)
          }
          LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
    } else {
          contains(QT_ARCH, i386) {
             HAMLIBDIR = $$absolute_path(../../hamlib-w32-$$HAMLIBVERSION)
          }
          contains(QT_ARCH, x86_64) {
             HAMLIBDIR = $$absolute_path(../../hamlib-w64-$$HAMLIBVERSION)
          }
          msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-4
          msvc: DEFINES += DLL_EXPORT
    }
    INCLUDEPATH += $$HAMLIBDIR/include
}

unix: {
   LIBS += -lhamlib
}

mac: {
   HAMLIBDIR = /usr/local
   LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
   INCLUDEPATH += $${HAMLIBDIR}/include
}

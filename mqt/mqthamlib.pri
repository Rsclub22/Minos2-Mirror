# Allow testing of pre-release hamlib library
# comment out HAMLIBVER to use the current release
# The files need to be in C:\Qt_Projects
# Also make sure the Windows Path statement contains the path to the Hamlib Bin Dir
# For example C:\Qt_Projects\hamlib-w32-4.3.1\bin


#HAMLIBVER = $$(hamlib)
#HAMLIBVER = $$(45)
#HAMLIBVER = 455
HAMLIBVER = 460
#message(The hamlib version is $$HAMLIBVER)

equals(HAMLIBVER, 455) {

    message(Using  hamlib V4.5.5)
    win32: {
       win32-g++* {
          contains(QT_ARCH, i386) {
          HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.5.5)
          message(Hamlib path is $$HAMLIBDIR)
          }
       contains(QT_ARCH, x86_64) {
          HAMLIBDIR = $$absolute_path(../../hamlib-w64-4.5.5)
          }
          LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
       } else {
          contains(QT_ARCH, i386) {
             HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.5.5)
          }
          contains(QT_ARCH, x86_64) {
             HAMLIBDIR = $$absolute_path(../../hamlib-w64-4.5.5)
          }
          msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-4
          msvc: DEFINES += DLL_EXPORT
       }
       INCLUDEPATH += $$HAMLIBDIR/include

    }
} else {
    message(Using test version - currently set at hamlib V4.6)
   win32: {
   win32-g++* {
      contains(QT_ARCH, i386) {
      HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.6)
      message(Hamlib path is $$HAMLIBDIR)
      }
   contains(QT_ARCH, x86_64) {
      HAMLIBDIR = $$absolute_path(../../hamlib-w64-4.6)
      }
      LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
   } else {
      contains(QT_ARCH, i386) {
         HAMLIBDIR = $$absolute_path(../../hamlib-w32-4.6)
      }
      contains(QT_ARCH, x86_64) {
         HAMLIBDIR = $$absolute_path(../../hamlib-w64-4.6)
      }
      msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-4
      msvc: DEFINES += DLL_EXPORT
    }

    INCLUDEPATH += $$HAMLIBDIR/include
    }
}


unix: {
   LIBS += -lhamlib
}

mac: {
   HAMLIBDIR = /usr/local
   LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
   INCLUDEPATH += $${HAMLIBDIR}/include
}

# This does not require the path to be set, the dll is copied to the debug/release
# directory. Only the Version number needs to be changed.

HAMLIBVERSION=4.7

DEFINES += HAMLIBVERSION=\\\"$$HAMLIBVERSION\\\"
# message(Using hamlib V$$HAMLIBVERSION)

win32: {
   contains(QT_ARCH, i386) {
   HAMLIBDIR = $$absolute_path(../../hamlib-w32-$$HAMLIBVERSION)
#    message(Hamlib path is $$HAMLIBDIR)
   }
   contains(QT_ARCH, x86_64) {
   HAMLIBDIR = $$absolute_path(../../hamlib-w64-$$HAMLIBVERSION)
   }
   INCLUDEPATH += $$HAMLIBDIR/include

   win32-g++* {
         LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
   } else: win32-clang-g++: {
       LIBS += -L$$HAMLIBDIR/lib/gcc/ -llibhamlib
   } else{
         msvc: LIBS += -L$$HAMLIBDIR/lib/msvc/ -llibhamlib-4
         msvc: DEFINES += DLL_EXPORT
   }

# Copy DLL to build directory
  CONFIG(debug, debug|release) {
       DESTDIR_DLL = $$OUT_PWD/debug
  } else {
       DESTDIR_DLL = $$OUT_PWD/release
  }

  HAMLIB_DLL = $$HAMLIBDIR/bin/libhamlib-4.dll
  QMAKE_POST_LINK += $$quote($$QMAKE_COPY $$shell_path($$HAMLIB_DLL) $$shell_path($$DESTDIR_DLL)$$escape_expand(\\n\\t))
}

unix: {
  LIBS += -lhamlib
}

mac: {
  HAMLIBDIR = /usr/local
  LIBS += -L$$HAMLIBDIR/lib/ -L$${HAMLIBDIR}/lib/hamlib -lhamlib
  INCLUDEPATH += $${HAMLIBDIR}/include
}

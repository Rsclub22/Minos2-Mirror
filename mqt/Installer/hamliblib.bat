call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cd C:\projects\hamlib-w32-4.7.2\lib\msvc
lib /def:libhamlib-4.def /machine:X86 /name:libhamlib-4



call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

rem cd
cd C:\projects\hamlib-w64-4.7.2\lib\msvc

rem lib
lib /def:libhamlib-4.def /machine:X64 /name:libhamlib-4

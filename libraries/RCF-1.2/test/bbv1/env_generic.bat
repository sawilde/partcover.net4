@echo off
set BOOST_ROOT=C:/xyz/boost_1_33_1
set OPENSSL_ROOT=C:/xyz/OpenSSL-0.9.8d/include
set ZLIB_ROOT=C:/xyz/zlib/include
set ALL_LOCATE_TARGET=C:/xyz/Builds/
set BUILD_CONFIG_TEMP_DIR=C:\\xyz\\RCF\\test\\data
set SOCKET_LIBRARY_1=ws2_32
REM set SOCKET_LIBRARY_2=
set OPENSSL_LIBRARY_1=libeay32MD
set OPENSSL_LIBRARY_2=ssleay32MD
set ZLIB_LIBRARY=zdll

REM Setup vc6
set MSVC_ROOT="C:\Program_Files\Microsoft Visual Studio\VC98"

REM Setup intel
REM set INTEL_BASE_MSVC_TOOLSET=vc-7_1
REM set INTEL_PATH="C:\Program_Files\Intel\Compiler70\IA32"
REM set INTEL_PATH="C:\Program_Files\Intel\CPP\Compiler80\Ia32\"

REM Setup metrowerks
set CW_ROOT="C:\Program_Files\Metrowerks\CodeWarrior"

REM Setup borland
set BCCROOT="C:\Program_Files\Borland\CBuilder6"

REM Setup mingw
set MINGW_3.2_ROOT_DIRECTORY="C:\Program_Files\mingw-gcc3.2"
set MINGW_3.3_ROOT_DIRECTORY="C:\Program_Files\mingw-gcc3.3"
set MINGW_3.4_ROOT_DIRECTORY="C:\Program_Files\mingw-gcc3.4"
set MINGW_2.95_ROOT_DIRECTORY="C:\Program_Files\mingw-gcc2.95"
set MINGW_4.1.2_ROOT_DIRECTORY="C:\Program_Files\mingw-gcc4.1.2"

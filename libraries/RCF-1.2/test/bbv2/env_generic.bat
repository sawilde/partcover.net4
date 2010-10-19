@echo off
set BOOST_ROOT=E:/jarl/boost_1_34_0
set BJAM=%BOOST_ROOT%\tools\jam\src\bin.ntx86\bjam.exe
set CMAKE=E:\jarl\cmake-2.6.1\bin\cmake.exe
set BUILD_DIR=
set BUILD_CONFIG_TEMP_DIR=E:\\jarl\\RCF\\test\\data
set CHECKOUT_NAME=RCF
set CONCURRENCY=12
set OPENSSL_ROOT=E:/jarl/OpenSSL-0.9.8d/include
set OPENSSL_NAME_1=libeay32MD.lib
set OPENSSL_NAME_2=ssleay32MD.lib
set ZLIB_ROOT=E:/jarl/zlib/include
set ZLIB_NAME=zdll.lib
set PROTOBUF_ROOT=E:/jarl/protobuf-2.1.0/src
set PROTOBUF_NAME_DEBUG=libprotobufD
set PROTOBUF_NAME_RELEASE=libprotobufR

set PATH=%PATH%;E:\Development\protobuf-2.1.0\vsprojects\Debug

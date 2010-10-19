#!/bin/sh

export BOOST_ROOT=/home/jarl/Development/boost_1_34_1
export BJAM=$BOOST_ROOT/tools/jam/src/bin.linuxx86/bjam
export BUILD_CONFIG_TEMP_DIR=/home/jarl/Development/RCF/test/data
export CONCURRENCY=1

# For bbv1
export SOCKET_LIBRARY_1=nsl
export OPENSSL_LIBRARY_1=crypto
export OPENSSL_LIBRARY_2=ssl
export ZLIB_LIBRARY=z

# For bbv1
export SOCKET_NAME_1=nsl
export OPENSSL_NAME_1=crypto
export OPENSSL_NAME_2=ssl
export ZLIB_NAME=z

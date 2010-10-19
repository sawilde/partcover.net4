
#ifndef RCF_SINGLE_THREADED
#error This test must be built with RCF in single-threaded mode, i.e. RCF_SINGLE_THREADED must be defined.
#endif

#define Test_CycleServer Test_CycleServer_St
#include "Test_CycleServer.cpp"

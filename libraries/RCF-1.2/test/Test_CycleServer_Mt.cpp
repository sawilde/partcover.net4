
#ifdef RCF_SINGLE_THREADED
#error This test must be built with RCF in multi-threaded mode.
#endif

#define Test_CycleServer Test_CycleServer_Mt
#include "Test_CycleServer.cpp"

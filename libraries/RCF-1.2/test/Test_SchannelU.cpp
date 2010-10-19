
#if !defined(UNICODE)  || !defined(_UNICODE)
#error Unicode needs to be enabled for this test
#endif

// this test will exercise inter-Windows std::Wstring marshaling functionality, since tstring = wstring

#include "Test_Schannel.cpp"

#include "StdAfx.h"
#include "helpers.h"

void ParseEnvironmentBlock(LPTSTR data, LPTSTR* op_varEnd, LPTSTR* op_valStart, LPTSTR* op_valEnd) {
    LPTSTR varEnd, valStart, valEnd;
    varEnd = data + 1;
    while(*varEnd != _T('=')) ++varEnd;
    valEnd = valStart = varEnd + 1;
    while(*valEnd != 0) ++valEnd;
    *op_varEnd = varEnd;
    *op_valStart = valStart;
    *op_valEnd = valEnd;
} 

StringMap ParseEnvironment() {
    LPTSTR data = ::GetEnvironmentStrings();
    StringMap result;

    LPTSTR current_token = data;
    do {
        LPTSTR varEnd, valStart, valEnd;    
        ParseEnvironmentBlock(current_token, &varEnd, &valStart, &valEnd);
        String name(current_token, varEnd - current_token);
        String value(valStart, valEnd - valStart);
        result.insert(StringMap::value_type(name, value));
        current_token = valEnd + 1;
    } while( *current_token != 0 );

    ::FreeEnvironmentStrings(data);
    return result;
}

LPTSTR CreateEnvironment(const StringMap& env) {
    size_t buffer_size = 0;
    // get whole buffer size
    for(StringMap::const_iterator it = env.begin(); it != env.end(); ++it)
        buffer_size += it->first.length() + 1 + it->second.length() + 1;
    ++buffer_size;

    // initialize buffer
    LPTSTR buffer = static_cast<LPTSTR>(malloc(sizeof(TCHAR) * (buffer_size + 1)));
	::ZeroMemory(buffer, sizeof(TCHAR)* (buffer_size+1));

    // store for format
    LPTSTR buffer_data = buffer;

    // copy data
    for(StringMap::const_iterator it = env.begin(); it != env.end(); ++it) {
		int written = _stprintf_s(buffer, buffer_size, _T("%s="), it->first.c_str());
        buffer += written;
		buffer_size -= written;

        written = 1 + _stprintf_s(buffer, buffer_size, _T("%s"), it->second.c_str());
        buffer += written;
		buffer_size -= written;
    }
    *buffer = 0;
    return buffer_data;
}

void FreeEnvironment( LPTSTR buffer ) {
    free(buffer);
}

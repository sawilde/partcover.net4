#include "StdAfx.h"
#include "messagecenter.h"
#include "helpers.h"

//#define TRACE_SEND_RECEIVE

LPCTSTR MESS_PIPID = _T("\\\\.\\pipe\\PartCover.MessageCenter");
const DWORD MESSAGE_BUFFER_SIZE = 1024;

MessageCenter::MessageCenter(void) : m_pipe(INVALID_HANDLE_VALUE) {}
MessageCenter::~MessageCenter(void) {}

LPCTSTR MessageCenter::getId() const {
    return (LPCTSTR) m_pipename;
}

HRESULT MessageCenter::Open() {
#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("MessageCenter::Open..."));
#endif
    ATLASSERT(m_pipe == INVALID_HANDLE_VALUE);

    // create pipe name
    TCHAR buffer[128];
	_stprintf_s(buffer, 128, _T("%s.%d.%d"), MESS_PIPID, GetCurrentProcessId(), GetTickCount());
    m_pipename = buffer;

    m_pipe = CreateNamedPipe(buffer, 
        PIPE_ACCESS_DUPLEX|FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,
        2,
        MESSAGE_BUFFER_SIZE,
        MESSAGE_BUFFER_SIZE,
        1000,
        NULL);

    if (m_pipe == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32( ::GetLastError() );

#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("MessageCenter::Open - S_OK"));
#endif
    return S_OK;
}

HRESULT MessageCenter::Connect(LPCTSTR pipeName) {
#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("MessageCenter::Connect to"), pipeName);
#endif

    ATLASSERT(m_pipe == INVALID_HANDLE_VALUE);
    m_pipe = CreateFile(
        pipeName, 
        GENERIC_READ|GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

     if (m_pipe == INVALID_HANDLE_VALUE) 
         return HRESULT_FROM_WIN32(::GetLastError());

#ifdef TRACE_SEND_RECEIVE
     ATLTRACE(_T("MessageCenter::Connect established"));
#endif
    return S_OK;
}

HRESULT MessageCenter::WaitForClient() {
#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("MessageCenter::WaitForClient..."));
#endif
    if ( ConnectNamedPipe(m_pipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
#ifdef TRACE_SEND_RECEIVE
        ATLTRACE(_T("MessageCenter::WaitForClient - Client Succesfully connected"));
#endif
        return S_OK;
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

HRESULT MessageCenter::ReceiveData(LPBYTE buffer, DWORD dwSize) {
    if (dwSize == 0) return S_OK;
    BOOL fSucess = FALSE;
    DWORD dwReaded = 0;
    while(dwSize > 0 && (fSucess = ReadFile(m_pipe, buffer, dwSize, &dwReaded, NULL))) {
        dwSize -= dwReaded;
        buffer += dwReaded;
        Sleep(0);
    }
    if (!fSucess) return HRESULT_FROM_WIN32(::GetLastError());
    if (dwReaded == 0) return E_ABORT;
    return S_OK;
}

HRESULT MessageCenter::WaitForOption( Message* option ) {
    if (!isOpen()) return E_ACCESSDENIED;

    HRESULT hr;
    // receive header
    Message::option_type code;
    if(FAILED(hr = ReceiveData(&code, sizeof code)))
        return hr;
    option->code = code;
    // receive value size
    size_t value_size = 0;
    if(FAILED(hr = ReceiveData((LPBYTE)&value_size, sizeof value_size)))
        return hr;

    STATIC_ASSERT(sizeof(size_t) == sizeof(DWORD));

	Message::byte_array::value_type buffer[1024];

    // receive value
	option->value.reserve(value_size);
	do
	{
		Message::byte_array::size_type toReceive = min(1024, value_size);
		value_size -= toReceive;

		hr = ReceiveData(buffer, toReceive);

		option->value.insert(option->value.end(), buffer, buffer + toReceive);
		
	} while (value_size > 0 && !FAILED(hr));

    return S_OK;
}

HRESULT MessageCenter::SendData(LPCBYTE buffer, DWORD dwSize) {
    if (dwSize == 0) return S_OK;
    BOOL fSucess = FALSE;
    DWORD sendedSize = 0;
    while(dwSize > 0 && (fSucess = WriteFile(m_pipe, buffer, dwSize, &sendedSize, NULL))) {
        dwSize -= sendedSize;
        buffer += sendedSize;
    }
    if (!fSucess) return HRESULT_FROM_WIN32(::GetLastError());
    if (sendedSize == 0) return E_ABORT;
    return S_OK;
}

HRESULT MessageCenter::SendOption( const Message& option ) {
    HRESULT hr;
#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("Send option %d"), option.code);
#endif
    ATLASSERT(isOpen());

    // send header
    if(FAILED(hr = SendData(&option.code, sizeof(option.code))))
        return hr;
    // send value size
    size_t valueSize = option.value.size();
    if(FAILED(hr = SendData((LPCBYTE)&valueSize, sizeof valueSize)))
        return hr;
    // send value
    STATIC_ASSERT(sizeof(size_t) == sizeof(DWORD));

	hr = S_OK;
	Message::byte_array::value_type buffer[1024];
	Message::byte_array::const_iterator value_it = option.value.begin();
	Message::byte_array::size_type value_size = option.value.size();

	do {
		Message::byte_array::size_type toCopy = min(value_size, 1024);
		value_size -= toCopy;

		std::copy(value_it, value_it + toCopy, buffer);
		value_it += toCopy;

		hr = SendData(buffer, toCopy);

	} while(value_size > 0 && !FAILED(hr));

    return hr;
}

HRESULT MessageCenter::SendOption( const Message::option_type& code ) {
    Message mess; mess.code = code;
    return SendOption(mess);
}

HRESULT MessageCenter::WaitForOption( const Message::option_type& code, Message* option ) {
    HRESULT hr;
    Message mess;
    while(SUCCEEDED(hr = WaitForOption(&mess))) {
        if (mess.code == code) {
            if (option != 0) {
                option->code = mess.code;
                option->value.swap(mess.value);
            }
            return hr;
        }
    }
    return hr;
}

Message::byte_array::iterator Message::pop(Message::byte_array::iterator it, std::wstring* value) {
    size_t stringSize;
    it = pop(it, &stringSize);
    value->resize(stringSize);
    for(size_t i = 0; i < stringSize; i++)
        it = pop(it, &(*value)[i]);
    return it;
}

void Message::push(Message::byte_array& array, std::wstring value) {
    push(array, value.size());
    for (std::wstring::const_iterator it = value.begin(); it != value.end(); ++it)
        push(array, *it);
}

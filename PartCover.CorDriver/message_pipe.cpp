#include "StdAfx.h"
#include "message.h"
#include "message_pipe.h"
#include "helpers.h"

//#define TRACE_SEND_RECEIVE

LPCTSTR MESS_PIPID = _T("\\\\.\\pipe\\PartCover.MessageCenter2");
const DWORD MESSAGE_BUFFER_SIZE = 1024;

MessagePipe::MessagePipe(void) : m_pipe(INVALID_HANDLE_VALUE) {}
MessagePipe::~MessagePipe(void) {}

LPCTSTR MessagePipe::getId() const {
    return (LPCTSTR) m_pipename;
}

HRESULT MessagePipe::Open() {
#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("MessageCenter::Open..."));
#endif
    ATLASSERT(m_pipe == INVALID_HANDLE_VALUE);

    // create pipe name
    TCHAR buffer[128];
	_stprintf_s(buffer, 128, _T("%s.%d.%d"), MESS_PIPID, GetCurrentProcessId(), GetTickCount());
    m_pipename = buffer;

    m_pipe = CreateNamedPipe(buffer, 
        PIPE_ACCESS_DUPLEX|FILE_FLAG_FIRST_PIPE_INSTANCE|FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,
        2,
        MESSAGE_BUFFER_SIZE,
        MESSAGE_BUFFER_SIZE,
        10000,
        NULL);

    if (m_pipe == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32( ::GetLastError() );

#ifdef TRACE_SEND_RECEIVE
    ATLTRACE(_T("MessageCenter::Open - S_OK"));
#endif
    return S_OK;
}

HRESULT MessagePipe::Connect(LPCTSTR pipeName) {
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

HRESULT MessagePipe::WaitForClient() {
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

HRESULT MessagePipe::SetMessageMap(ITransferrableFactory* map[], size_t mapLen)
{
	for(size_t i = 0; i < mapLen; ++i)
		m_factories[i] = map[i];
	return S_OK;
}

ITransferrable* MessagePipe::ResolveMessage(MessageType type)
{
	ITransferrable* res = 0;
	for(size_t i = 0; res == 0 && i < ARRAYSIZE(m_factories); ++i) {
		if (m_factories[i] == 0)
			break;
		res = m_factories[i]->create(type);
	}
	return res;
}

HRESULT MessagePipe::Wait(ITransferrable* &message) 
{
    HRESULT hr = WaitHeader(message);
	if (FAILED(hr))
		return hr;
	return message->ReceiveData(*this) ? S_OK : E_FAIL;
}

HRESULT MessagePipe::WaitHeader(ITransferrable* &message) 
{
	STATIC_ASSERT(sizeof(MessageType) == sizeof(int));

    if (!isOpen()) return E_ACCESSDENIED;

    HRESULT hr;
    // receive header
	int code;
    if(FAILED(hr = read(&code, sizeof code)))
        return hr;

	message = ResolveMessage(static_cast<MessageType>(code));
	return message == 0 ? E_FAIL : S_OK;
}

HRESULT MessagePipe::Send(ITransferrable &message)
{
	if (!write(message.getType()))
		return false;
	return message.SendData(*this);
}

HRESULT MessagePipe::read(void* bufferPtr, int bufferLen)
{
    if (bufferLen == 0) return S_OK;

	int bufferRead = 0;
	char* buffer = static_cast<char*>(bufferPtr);

    BOOL fSucess = FALSE;
    DWORD dwReaded = 0;
    while(bufferRead < bufferLen && (fSucess = ReadFile(m_pipe, buffer, bufferLen - bufferRead, &dwReaded, NULL))) 
	{
        bufferRead += dwReaded;
        buffer += dwReaded;
        Sleep(0);
    }
    if (!fSucess) 
		return HRESULT_FROM_WIN32(::GetLastError());
    if (dwReaded == 0) 
		return E_ABORT;
    return S_OK;
}

HRESULT MessagePipe::write(const void* bufferPtr, int bufferLen) {
    if (bufferLen == 0) return S_OK;

	const char* buffer = static_cast<const char*>(bufferPtr);
	int bufferSent = 0;

    BOOL fSucess = FALSE;
    DWORD sendedSize = 0;

    while(bufferSent < bufferLen && (fSucess = WriteFile(m_pipe, buffer, bufferLen - bufferSent, &sendedSize, NULL))) {
        bufferSent += sendedSize;
        buffer += sendedSize;
    }

    if (!fSucess) return HRESULT_FROM_WIN32(::GetLastError());
    if (sendedSize == 0) return E_ABORT;
    return S_OK;
}

bool MessagePipe::write(String value)
{
	size_t len = value.size();
	if (FAILED(write(&len, sizeof(len))))
		return false;
	return SUCCEEDED(write(value.c_str(), static_cast<int>(len * sizeof(String::value_type))));
}

bool MessagePipe::read(String* value)
{
	size_t len;
	if (FAILED(read(&len, sizeof(len))))
		return false;

	DynamicArray<String::value_type> data(len);
	if (FAILED(read(data, static_cast<int>(len * sizeof(String::value_type)))))
		return false;

	value->assign(data, len);
	return true;
}


bool LogMessage::SendData(MessagePipe &pipe) {
	return 
		pipe.write(this->m_threadId) && 
		pipe.write(this->m_tick) && 
		pipe.write(this->m_message);
}

bool LogMessage::ReceiveData(MessagePipe &pipe) {
	return 
		pipe.read(&this->m_threadId) &&
		pipe.read(&this->m_tick) &&
		pipe.read(&this->m_message);
}
#pragma once

class MessagePipe
{
    _bstr_t m_pipename;
    HANDLE  m_pipe;

	ITransferrableFactory* m_factories[100];

	MessagePipe(const MessagePipe& pipe);
	
	HRESULT read(void* buffer, int dwSize);
	HRESULT write(const void* buffer, int dwSize);

	ITransferrable* ResolveMessage(MessageType type);

	HRESULT WaitHeader(ITransferrable* &message);

public:
    MessagePipe(void);
    ~MessagePipe(void);

    HRESULT Open();
    HRESULT Connect(LPCTSTR pipeName);
    HRESULT WaitForClient();

    LPCTSTR getId() const;
    bool isOpen() const;

	template<typename T> bool write(T value) { return SUCCEEDED(write(&value, sizeof T)); }
	template<typename T> bool read(T* value) { return SUCCEEDED(read(value, sizeof T)); }

	bool write(String value);
	bool read(String* value);


public:

	HRESULT SetMessageMap(ITransferrableFactory* map[], size_t mapLen);

	HRESULT Wait(ITransferrable* &message);

	HRESULT Send(ITransferrable &message);
};

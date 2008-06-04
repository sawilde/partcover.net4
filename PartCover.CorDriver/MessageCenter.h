#pragma once

struct Message {
    typedef unsigned char option_type;
    typedef std::vector<unsigned char> byte_array;

    option_type code;
    byte_array  value;

    template<typename value_t> static void push(Message::byte_array& array, value_t value) {
        typedef const Message::byte_array::value_type* value_ptr;
        value_ptr dataSizePtr = (value_ptr) &value;
        array.insert(array.end(), dataSizePtr, dataSizePtr + sizeof(value_t));
    }

    template<typename value_t> static Message::byte_array::iterator pop(Message::byte_array::iterator it, value_t* value) {
        typedef Message::byte_array::value_type* value_ptr;
        *value = *((value_t*)&(*it));
        return it + sizeof(value_t);
    }

    static Message::byte_array::iterator pop(Message::byte_array::iterator it, std::wstring* value);
    static void push(Message::byte_array& array, std::wstring value);
};

class MessageCenter
{
    _bstr_t m_pipename;
    HANDLE  m_pipe;

public:
    MessageCenter(void);
    ~MessageCenter(void);

    HRESULT Open();
    HRESULT Connect(LPCTSTR pipeName);
    HRESULT WaitForClient();

    LPCTSTR getId() const;
    bool isOpen() const { return m_pipe != INVALID_HANDLE_VALUE; }

    HRESULT SendData(LPCBYTE buffer, DWORD dwSize);
    HRESULT ReceiveData(LPBYTE buffer, DWORD dwSize);
public:

    HRESULT SendOption( const Message& option );
    HRESULT SendOption( const Message::option_type& code );

    HRESULT WaitForOption( Message* option );
    HRESULT WaitForOption( const Message::option_type& code, Message* option = 0 );
};

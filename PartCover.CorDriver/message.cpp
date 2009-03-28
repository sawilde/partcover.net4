#include "stdafx.h"
#include "message.h"

namespace Messages 
{
	GenericMessage::GenericMessage(MessageType type) : m_type(type) 
	{
	}

	MessageType GenericMessage::getType() const 
	{ 
		return m_type; 
	}

	bool GenericMessage::SendData(MessagePipe&) 
	{
		return true; 
	}

	bool GenericMessage::ReceiveData(MessagePipe&) 
	{
		return true; 
	}

	void GenericMessage::visit(ITransferrableVisitor& visitor) 
	{
		visitor.on(getType()); 
	}
}

LogMessage::LogMessage(String message, long tick) 
	: m_message(message), m_tick(tick)
{
	m_threadId = ::GetCurrentThreadId();
}

LogMessage::LogMessage() 
	: m_threadId(0), m_tick(0) 
{
}

void LogMessage::visit(ITransferrableVisitor& visitor) 
{
	visitor.on(*this); 
}

int LogMessage::getThreadId() const 
{ 
	return m_threadId; 
}

long LogMessage::getTicks() const 
{
	return m_tick; 
}

const String& LogMessage::getMessage() const 
{
	return m_message; 
}

#pragma once

class MessagePipe;
/*
struct Message {
public:
	static const int Rules = 100;
	static const int FunctionMap = 101;
	static const int InstrumentResults = 102;
};
*/
typedef int MessageType;

class FunctionMap;
class Rules;
class InstrumentResults;
struct LogMessage;

interface ITransferrableVisitor {
public:
	virtual void on(MessageType type) = 0;
	virtual void on(FunctionMap& value) = 0;
	virtual void on(Rules& value) = 0;
	virtual void on(InstrumentResults& value) = 0;
	virtual void on(LogMessage& value) = 0;
};

interface ITransferrable {
	virtual ~ITransferrable() {}

	virtual MessageType getType() const = 0;
    virtual bool SendData(MessagePipe&) = 0;
    virtual bool ReceiveData(MessagePipe&) = 0;

	virtual void visit(ITransferrableVisitor& visitor) = 0;
};

interface ITransferrableFactory {
	virtual ITransferrable* create(MessageType type) = 0;
};

namespace Messages {
	const MessageType C_RequestStart = 1;

	const MessageType C_RequestShutdown = 2;
	const MessageType C_RequestClose = 3;

	const MessageType C_EndOfInputs = 4;
	const MessageType C_EndOfResults = 5;

	const MessageType C_Rules = 100;
	const MessageType C_FunctionMap = 101;
	const MessageType C_InstrumentResults = 102;

	const MessageType C_LogMessage = 10;

	struct GenericMessage : public ITransferrable
	{
		const MessageType m_type;
	public:
		GenericMessage(MessageType type) : m_type(type) {}

		MessageType getType() const { return m_type; }
		bool SendData(MessagePipe&) { return true; }
		bool ReceiveData(MessagePipe&) { return true; }
		void visit(ITransferrableVisitor& visitor) { visitor.on(getType()); }
	};

	template<MessageType CODE> struct Message : public ITransferrable
	{
	public:
		MessageType getType() const { return CODE; }
		bool SendData(MessagePipe&) { return true; }
		bool ReceiveData(MessagePipe&) { return true; }
		void visit(ITransferrableVisitor& visitor) { visitor.on(getType()); }
	};
}

struct LogMessage : public Messages::Message<Messages::C_LogMessage>
{
public:
	LogMessage(String message, long tick) : m_message(message), m_tick(tick)
	{
		m_threadId = ::GetCurrentThreadId();
	}

	LogMessage() : m_threadId(0), m_tick(0) {}

	bool SendData(MessagePipe& pipe);
	bool ReceiveData(MessagePipe&);
	void visit(ITransferrableVisitor& visitor) { visitor.on(*this); }

	int getThreadId() const { return m_threadId; }
	long getTicks() const { return m_tick; }
	const String& getMessage() const { return m_message; }

private:
	int m_threadId;
	long m_tick;
	String m_message;
};


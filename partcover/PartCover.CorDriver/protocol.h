#pragma once

#ifndef PARTCOVER_PROTOCOL
#define PARTCOVER_PROTOCOL


#include "RPCLib/proxy.h"
#include "RPCLib/rpcserver.h"
#include "RPCLib/pipestransport.h"
#include "RPCLib/transport/transport.h"

class Rules;
class FunctionMap;
class InstrumentResults;

namespace bidi
{
	NamedPipe & operator << (NamedPipe &ar, const Rules &t);
	NamedPipe & operator >> (NamedPipe &ar, Rules  &t);

	NamedPipe & operator << (NamedPipe &ar, const FunctionMap &t);
	NamedPipe & operator >> (NamedPipe &ar, FunctionMap  &t);

	NamedPipe & operator << (NamedPipe &ar, const InstrumentResults &t);
	NamedPipe & operator >> (NamedPipe &ar, InstrumentResults  &t);
}

namespace DriverState 
{
	enum {
		Unknown = 0,
		Starting,
		Running,
		Stopping,
		Stopped
	};
}

RPC_LIBRARY
(
	PartCoverRpc, 0,

	RPC_IMPORT(RPC_UNKNOWN)

	RPC_INTERFACE
	(
		IIntercommunication,
		IRPCUnknown,
		RPC_METHOD(GetSettings, rpclib::RPCRESULT, (int))
		RPC_METHOD(GetRules, rpclib::RPCRESULT, ((RPC_OUT)Rules*))
		RPC_METHOD(SetDriverState, rpclib::RPCRESULT, (int))
		RPC_METHOD(AddFunction, rpclib::RPCRESULT, (int)(String)(String))
		RPC_METHOD(LogMessage, rpclib::RPCRESULT, (DWORD)(DWORD)(String))

		RPC_METHOD(StoreResultFunctionMap, rpclib::RPCRESULT, (FunctionMap&))
		RPC_METHOD(StoreResultInstrumentation, rpclib::RPCRESULT, (InstrumentResults&))
	),

	RPC_COCLASS(IntercommunicationProxy, IIntercommunication)
)

struct IntercommunicationProxy : public rpclib::std_coclass<IIntercommunication>
{
public:
	static IntercommunicationProxy instance;

	IIntercommunication* aggregate;

	virtual rpclib::RPCRESULT LogMessage(DWORD thread, DWORD tick, String message)
	{
		if (aggregate != 0) 
			return aggregate->LogMessage(thread, tick, message);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT GetSettings(int type)
	{
		if (aggregate != 0) 
			return aggregate->GetSettings(type);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT SetDriverState(int type)
	{
		if (aggregate != 0) 
			return aggregate->SetDriverState(type);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT AddFunction(int id, String param1, String param2)
	{
		if (aggregate != 0) 
			return aggregate->AddFunction(id, param1, param2);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT GetRules(Rules *rules)
	{
		if (aggregate != 0) 
			return aggregate->GetRules(rules);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT IIntercommunication::StoreResultFunctionMap(FunctionMap &map)
	{
		if (aggregate != 0) 
			return aggregate->StoreResultFunctionMap(map);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT IIntercommunication::StoreResultInstrumentation(InstrumentResults &map)
	{
		if (aggregate != 0) 
			return aggregate->StoreResultInstrumentation(map);
		return RPC_S_ACCESS_DENIED;
	}

	virtual rpclib::RPCRESULT  Release(void) 
	{
		return RPC_S_OK;
	}
};

RPC_BEGIN_FACTORY(PartCoverRpcObjectFactory)
	RPC_STATIC_OBJECT_ENTRY(IntercommunicationProxy, IIntercommunication, IntercommunicationProxy::instance)
RPC_END_FACTORY()

typedef bidi::NamedPipe TransportStream;
typedef bidi::BidiArchive<TransportStream> Transport;

typedef 
	mtlib::ThreadedServer<
		mtlib::SynchronousServer<
			PipeServer, 
			rpclib::StdDispatcher<
				PartCoverRpc<>::template Factory<Transport, PartCoverRpcObjectFactory>, 
				TransportStream
			> 
		> 
	> PartCoverMessageServer;

struct PartCoverMessageClient : public IntercommunicationProxy
{
public:
	PartCoverMessageClient() 
	{
	}

	bool Connect(LPCTSTR pipename) 
	{
		if (!m_client.open(pipename))
			return false;

		PartCoverRpc<>::CreateObject(m_client, 
			RPC_TYPE_REG(IntercommunicationProxy), &m_proxy);

		if (m_proxy != 0) {
			aggregate = m_proxy;
		}

		return m_proxy != 0;
	}

	void Disconnect() 
	{
		m_client.close();
	}

private:
	
	bidi::NamedPipe m_client;
	rpclib::CRpcPtr<IIntercommunication> m_proxy;
};

#endif
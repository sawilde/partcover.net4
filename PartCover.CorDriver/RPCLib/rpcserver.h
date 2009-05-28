//	rpcserver.h: rpc server library
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__RPCSERVER_H__
#define __RPCSERVER_H__

#include "mt/SynchronousServer.h"

namespace rpclib
{

template <class Factory, class Client>
struct StdDispatcher: public Client
{
	unsigned Run()
	{
		typename Factory::archive ar(*this);
		try
		{
			while (TRUE)
				Factory::Dispatch(ar);
		}
		catch (...)
		{
			return Factory::exception_filter::Filter(ar);
		}
	}
};

}

#endif // __RPCSERVER_H__

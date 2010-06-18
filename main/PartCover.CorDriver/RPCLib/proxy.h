//	proxy.h:		rpclib proxy definition
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__RPCLIB_PROXY_H__
#define __RPCLIB_PROXY_H__

#ifndef RPC_NO_VTABLE
#define RPC_NO_VTABLE __declspec(novtable)
#endif

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_void.hpp>
#include <boost/call_traits.hpp>
#include <cassert>

#include "idl.h"
#include "parameters.h"

#define RPC_PARAM_DIRECTION_AUTO	parameters::none
#define RPC_PARAM_DIRECTION_IN		parameters::in
#define RPC_PARAM_DIRECTION_OUT		parameters::out
#define RPC_PARAM_DIRECTION_INOUT	parameters::inout


namespace rpclib
{

struct CommonExceptionFilter
{
	template <class Archive>
	static RPCRESULT Filter(Archive &ar)
	{
/*		RPCRESULT nRes;
		try
		{
			throw
		}
		catch (...)
		{
			nRes = RPC_E_NETWORK_ERROR;
		}*/
		return RPC_E_NETWORK_ERROR;
	}
};

} // namespace rpclib


#define RPC_SERIALIZE_SEND Store
#define RPC_SERIALIZE_RECV Read


template <typename T>
struct serialize_traits_ex_client
{
	template <class P,  parameters::param_direction nDirection>
	struct inner:public parameters::serialize_traits_ex<T, nDirection, false>
	{
		typedef P type;
	};
};


#define RPC_SERIALIZE_PARAM_CLIENT(r, op, i, direction, type) serialize_traits_ex_client<type>::inner<this_type, direction>::op(m_ar, PPC_CAT(P, i));
#define RPC_SERIALIZE_PARAMS_CLIENT(params, op) RPC_FOR_EACH_PARAM(RPC_SERIALIZE_PARAM_CLIENT, op, params)

#define RPC_DECLARE_CLIENT_PROXY(id, name, calltype, ret, params, pre, post) \
	virtual ret calltype() name(RPC_ENUM_PARAMS_N(params, P))\
	{\
		pre()\
		ret valRet;\
		try\
		{\
			m_ar << m_tok << (rpclib::RPCIID) (id + ((RPC_ID(base_type) + 1) * PPC_MAX_REPEAT));\
			RPC_SERIALIZE_PARAMS_CLIENT(params, RPC_SERIALIZE_SEND)\
			m_ar.flush();\
			m_ar >> valRet;\
			RPC_SERIALIZE_PARAMS_CLIENT(params, RPC_SERIALIZE_RECV)\
		}\
		catch(...)\
		{\
			valRet = ExceptionFilter::Filter(m_ar);\
		}\
		post()\
		return valRet;\
	}


/*
#define RPC_DECLARE_CLIENT_PROXY(id, name, ret, params, pre, post) \
	virtual ret name(RPC_ENUM_PARAMS_N(params, P))\
	{\
		return PPC_CAT(method_proxy, name)((rpclib::RPCIID) (id + ((RPC_ID(base_type) + 1) * PPC_MAX_REPEAT)) PPC_COMMA_IF(RPC_PARAM_COUNT(params)) PPC_REPEAT_PARAMS(RPC_PARAM_COUNT(params), P));\
	}\
	ret PPC_CAT(method_proxy, name)(rpclib::RPCREFIID nId PPC_COMMA_IF(RPC_PARAM_COUNT(params)) RPC_ENUM_PARAMS_N(params, P))\
	{\
		pre()\
		ret valRet;\
		try\
		{\
			m_ar << m_tok << (rpclib::RPCIID)nId;\
			RPC_SERIALIZE_PARAMS_CLIENT(params, RPC_SERIALIZE_SEND)\
			m_ar.flush();\
			m_ar >> valRet;\
			RPC_SERIALIZE_PARAMS_CLIENT(params, RPC_SERIALIZE_RECV)\
		}\
		catch(...)\
		{\
			valRet = ExceptionFilter::Filter(m_ar);\
		}\
		post()\
		return valRet;\
	}
*/

#define RPC_CLIENT_PROXY(name) PPC_CAT(name, ClientProxy)

namespace rpclib
{

//  base client proxy 


template <class Archive, class I>
struct RPC_CLIENT_PROXY(IEmptyBase): public I
{
	RPC_CLIENT_PROXY(IEmptyBase)(Archive &ar, const Token t): m_ar(ar), m_tok(t){}
	virtual ~RPC_CLIENT_PROXY(IEmptyBase)()
	{
	}


/*	Archive &GetArchive()
	{
		return m_ar;
	}
	Archive &GetArchive() const
	{
		return m_ar;
	}
	Token GetToken() const
	{
		return m_tok;
	}
	void SetToken(const Token t)
	{
		m_tok = t;
	}

protected:*/
	Token m_tok;
	Archive &m_ar;
};



} // namespace rpclib


#define RPC_DECLARE_CLIENT_PROXY_IFS(id, name, parent, methods)\
template <class Archive, class I>\
struct RPC_CLIENT_PROXY(name): public RPC_CLIENT_PROXY(parent)<Archive, I>\
{\
	typedef lib_type library_type;\
	typedef name base_type;\
	typedef RPC_CLIENT_PROXY(name) this_type;\
	RPC_CLIENT_PROXY(name)(Archive &ar, const rpclib::Token t):RPC_CLIENT_PROXY(parent)<Archive, I>(ar, t){}\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_METHODS_IFS, RPC_DECLARE_CLIENT_PROXY, methods)\
};


#define RPC_DECLARE_PARAM(r, param, i, direction, t) parameters::type_traits<t>::type PPC_CAT(param, i);

#define RPC_SERIALIZE_PARAM_SERVER(r, op, i, direction, type) parameters::serialize_traits_ex<type, direction, true>::op(ar, PPC_CAT(P, i));
#define RPC_SERIALIZE_PARAMS_SERVER(params, op) RPC_FOR_EACH_PARAM(RPC_SERIALIZE_PARAM_SERVER, op, params)

#define RPC_SERVER_PROXY_FUN(name, id) PPC_CAT(name, PPC_CAT(ServerProxy, id))

#define RPC_DECLARE_SERVER_PROXY(id, name, calltype, ret, params, pre, post)\
	static void RPC_SERVER_PROXY_FUN(name, id)(rpclib::Token pObject, Archive &ar)\
	{\
		RPC_FOR_EACH_PARAM(RPC_DECLARE_PARAM, P, params)\
		RPC_SERIALIZE_PARAMS_SERVER(params, RPC_SERIALIZE_RECV)\
		ret valRet = ((base_type *)pObject)->name(PPC_REPEAT_PARAMS(RPC_PARAM_COUNT(params), P));\
		ar << valRet;\
		RPC_SERIALIZE_PARAMS_SERVER(params, RPC_SERIALIZE_SEND)\
		ar.flush();\
	}


#define RPC_DECLARE_SERVER_PROXY_TABLE(id, name, calltype, ret, params, pre, post) PPC_COMMA_IF(id) &RPC_SERVER_PROXY_FUN(name, id)

#define RPC_SERVER_PROXY(name) PPC_CAT(name, ServerProxy)
	
#define RPC_DECLARE_SERVER_PROXY_IFS(id, name, parent, methods)\
template <class Archive>\
struct RPC_SERVER_PROXY(name)\
{\
	typedef  name base_type;\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_METHODS_IFS, RPC_DECLARE_SERVER_PROXY, methods)\
	typedef void (*proxy_fun)(rpclib::Token, Archive &);\
	static proxy_fun proxy_table[];\
};\
template <class Archive>\
typename RPC_SERVER_PROXY(name)<Archive>::proxy_fun RPC_SERVER_PROXY(name)<Archive>::proxy_table[]=\
{\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_METHODS_IFS, RPC_DECLARE_SERVER_PROXY_TABLE, methods)\
};


#define RPC_IMPLEMENT_PROXY(id, name, parent, methods) \
RPC_DECLARE_CLIENT_PROXY_IFS(id, name, parent, methods)


#define RPC_DECLARE_SERVER_DISPATCH_TABLE(id, name, parent, _)  PPC_COMMA_IF(id) RPC_SERVER_PROXY(name)<Archive>::proxy_table
#define RPC_CREATE_CLIENT_OBJECT(id, name, parent, _)  case id: pObject = new RPC_CLIENT_PROXY(name)<Archive, name>(ar, nObject); break;


#define RPC_LIBRARY_FACTORY(name) PPC_CAT(name, Factory)

#define RPC_LIBRARY(name, id, ifaces, coclasses) \
RPC_LIBRARY_DECLARE_IFS(name, id, ifaces, coclasses)\
namespace rpclib {\
PPC_SEQ_FOR_EACH_I(RPC_ENUM_INTERFACES, RPC_DECLARE_SERVER_PROXY_IFS, ifaces)\
template <class Archive, class Factory>\
struct RPC_LIBRARY_FACTORY(name)\
{\
	typedef Archive archive;\
	static void CreateServerObject(rpclib::Token pObject, Archive &ar)\
	{\
		ar << (rpclib::Token)Factory::CreateObject((rpclib::RPCIID)pObject);\
		ar.flush();\
	}\
	static void Dispatch(Archive &ar)\
	{\
		rpclib::RPCIID nId;\
		rpclib::Token pObject;\
		ar >> pObject >> nId;\
		dispatch_table[nId/PPC_MAX_REPEAT][nId % PPC_MAX_REPEAT](pObject, ar);\
	}\
	typedef void (*proxy_fun)(rpclib::Token, Archive &);\
	typedef proxy_fun *dispatch_fun;\
	static proxy_fun creator;\
	static dispatch_fun dispatch_table[];\
};\
template <class Archive, class Factory>\
typename RPC_LIBRARY_FACTORY(name)<Archive, Factory>::proxy_fun RPC_LIBRARY_FACTORY(name)<Archive, Factory>::creator = CreateServerObject;\
template <class Archive, class Factory>\
typename RPC_LIBRARY_FACTORY(name)<Archive, Factory>::dispatch_fun RPC_LIBRARY_FACTORY(name)<Archive, Factory>::dispatch_table[]=\
{\
	&creator,\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_INTERFACES, RPC_DECLARE_SERVER_DISPATCH_TABLE, ifaces)\
};\
}\
template <class ExceptionFilter = rpclib::CommonExceptionFilter>\
struct name\
{\
	typedef name<ExceptionFilter> this_type;\
	typedef this_type lib_type;\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_INTERFACES, RPC_IMPLEMENT_PROXY, ifaces)\
	template <class Archive, class T>\
	static rpclib::RPCRESULT CreateObject(Archive &ar, rpclib::RPCREFCLSID idCoClass, T **ppInterface)\
	{\
		return CreateObject(ar, idCoClass, RPC_ID(T), (LPVOID *)ppInterface);\
	}\
	template <class Archive, class T, class C>\
	static rpclib::RPCRESULT CreateObject(Archive &ar, const C &, T **ppInterface)\
	{\
		RPC_VERIFY_COCLASS_REG(C::type, T);\
		return CreateObject(ar, C::ID, RPC_ID(T), (LPVOID *)ppInterface);\
	}\
	template <class Archive>\
	static rpclib::RPCRESULT CreateObject(Archive &ar, rpclib::RPCREFCLSID idCoClass, rpclib::RPCREFIID idInterface, LPVOID *ppInstance)\
	{\
		rpclib::Token nObject;\
		try\
		{\
			ar << (rpclib::Token)idCoClass << (unsigned int)0;\
			ar.flush();\
			ar >> nObject;\
		}\
		catch(...)\
		{\
			return ExceptionFilter::Filter(ar);\
		}\
		return ((*ppInstance = CreateClientObject(ar, nObject, idInterface))) ? RPC_S_OK : RPC_E_FAIL;\
	}\
	template <class Archive>\
	static void *CreateClientObject(Archive &ar, rpclib::Token nObject, rpclib::RPCREFIID nId)\
	{\
		void *pObject = NULL;\
		switch (nId)\
		{\
			PPC_SEQ_FOR_EACH_I(RPC_ENUM_INTERFACES, RPC_CREATE_CLIENT_OBJECT, ifaces)\
		}\
		return pObject;\
	}\
	template <class Archive, class F>\
	struct Factory: public rpclib::RPC_LIBRARY_FACTORY(name)<Archive, F>\
	{\
		typedef ExceptionFilter exception_filter;\
	};\
};

#include "ifs_parameters.h"

#endif

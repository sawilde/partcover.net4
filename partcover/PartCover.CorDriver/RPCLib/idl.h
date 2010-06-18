//	idl.h:		rpclib idl definition
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__RPCLIB_IDL_H__
#define __RPCLIB_IDL_H__

#ifndef RPC_NO_VTABLE
#define RPC_NO_VTABLE __declspec(novtable)
#endif

#define RPC_USE_BOOST_PP

#ifdef RPC_USE_BOOST_PP
//  tune preprocessor and meta library
#define PPC_USE_BOOST_PP
#endif

#include "types.h"
#include "preproc.h"
#include "parameters.h"


namespace rpclib
{


struct RPC_NO_VTABLE IEmptyBase
{
};

} // namespace rpclib


#define RPC_PARAM_RPC_PARAM_DIRECTION			RPC_PARAM_DIRECTION_AUTO,
#define RPC_PARAM_RPC_PARAM_DIRECTION_RPC_IN	RPC_PARAM_DIRECTION_IN
#define RPC_PARAM_RPC_PARAM_DIRECTION_RPC_OUT	RPC_PARAM_DIRECTION_OUT
#define RPC_PARAM_RPC_PARAM_DIRECTION_RPC_INOUT RPC_PARAM_DIRECTION_INOUT

#define RPC_PARAM_DIRECTION(x) PPC_CAT(RPC_PARAM_DIRECTION_, x),
#define RPC_PARAM_WITH_DIRECTION(x) PPC_CAT(RPC_PARAM_, RPC_PARAM_DIRECTION x)



// we must handle firs dummy parameter

#define RPC_PREP_PARAMS(params) PPC_SEQ_POP_FRONT(params)
#define RPC_PARAM_COUNT(params) PPC_DEC(PPC_SEQ_SIZE(params))
//#define RPC_PARAM_COUNT(params) PPC_SEQ_SIZE(RPC_PREP_PARAMS(params))
#define RPC_EMPTY_PARAM1(x)
#define RPC_EMPTY_PARAM2(x1, x2)
#define RPC_EMPTY_PARAM3(x1, x2, x3)

#define RPC_EMPTY_PARAM


// for each param called macro: macro(rest, data, index, direction, type)

#define RPC_FOR_EACH_PARAM_ITEM1(rest, data, index, elem) (rest, data, index, elem)
#define RPC_FOR_EACH_PARAM_ITEM_MACRO(macro, data) macro
#define RPC_FOR_EACH_PARAM_ITEM_DATA(macro, data) data
#define RPC_FOR_EACH_PARAM_ITEM(rest, data, index, elem) RPC_FOR_EACH_PARAM_ITEM_MACRO data RPC_FOR_EACH_PARAM_ITEM1(rest, RPC_FOR_EACH_PARAM_ITEM_DATA data, index, RPC_PARAM_WITH_DIRECTION(elem))
#define RPC_FOR_EACH_PARAM(macro, data, params) PPC_IF(RPC_PARAM_COUNT(params), PPC_SEQ_FOR_EACH_I, RPC_EMPTY_PARAM3)(RPC_FOR_EACH_PARAM_ITEM, (macro, data), RPC_PREP_PARAMS(params))

#define RPC_ENUM_PARAMS_ITEM(rest, data, index, direction, type) PPC_COMMA_IF(index) type
#define RPC_ENUM_PARAMS(params) RPC_FOR_EACH_PARAM(RPC_ENUM_PARAMS_ITEM, dummy, params)

#define RPC_ENUM_PARAMS_N_ITEM(rest, data, index, direction, type) PPC_COMMA_IF(index) type PPC_CAT(data, index)
#define RPC_ENUM_PARAMS_N(params, name) RPC_FOR_EACH_PARAM(RPC_ENUM_PARAMS_N_ITEM, name, params) 



#define RPC_DECLARE_METHOD_I1(name,  calltype, ret, params, pre, post) name, calltype, ret, params, pre, post
#define RPC_DECLARE_METHOD_I(id, method) (id, method)
#define RPC_ENUM_METHODS_IFS(r, data, id, method)\
	 data RPC_DECLARE_METHOD_I(id, RPC_DECLARE_METHOD_I1 method)


#define RPC_DECLARE_METHOD(id,  name, calltype, ret, params, pre, post)\
	virtual ret calltype() name(RPC_ENUM_PARAMS(params)) = 0;


#define RPC_DECLARE_INTERFACE(id, name, parent, methods)\
struct RPC_NO_VTABLE name: public parent\
{\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_METHODS_IFS, RPC_DECLARE_METHOD, methods)\
};\
RPC_DECLARE_GUID(name, id)\
RPC_DECLARE_IFS_PARAMETERS_SPECIALIZATION(name)

#define RPC_RELEASE_EPILOG() delete this;

#define RPC_METHOD_EX(name, calltype, ret, params, pre,  post) ((name,  calltype PPC_EMPTY, ret, (void) params, pre, post))
#define RPC_METHOD(name, ret, params) RPC_METHOD_EX(name, RPC_METHOD_CALL_TYPE, ret, RPC_EMPTY_PARAM params, PPC_EMPTY, PPC_EMPTY)
#define RPC_INTERFACE(name, parent, methods) ((name, parent, methods))
#define RPC_COCLASS(name, iface) ((name, iface))
#define RPC_IMPORT(iface_list) iface_list

#define RPC_UNKNOWN RPC_INTERFACE(IRPCUnknown, rpclib::IEmptyBase, RPC_METHOD_EX(Release, RPC_METHOD_CALL_TYPE, rpclib::RPCRESULT, RPC_EMPTY_PARAM, PPC_EMPTY, RPC_RELEASE_EPILOG))


#define RPC_DECLARE_COCLASS_I1(name, iface) name, iface
#define RPC_DECLARE_COCLASS_I(id, coclass) (id, coclass)
#define RPC_ENUM_COCLASSES(r, data, id, coclass)\
	 data RPC_DECLARE_COCLASS_I(id, RPC_DECLARE_COCLASS_I1 coclass)

#define RPC_DECLARE_COCLASS(id, name, iface)\
	struct name;\
	RPC_DECLARE_GUID(name, id)\
	RPC_REGISTER_COCLASS(name, iface)



#define RPC_DECLARE_INTERFACE_I1(name, parent, methods) name, parent, methods
#define RPC_DECLARE_INTERFACE_I(id, iface) (id, iface)
#define RPC_ENUM_INTERFACES(r, data, id, iface)\
	 data RPC_DECLARE_INTERFACE_I(id, RPC_DECLARE_INTERFACE_I1 iface)



#define RPC_LIBRARY_DECLARE_IFS(name, id, ifaces, coclasses) \
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_INTERFACES, RPC_DECLARE_INTERFACE, ifaces)\
	PPC_SEQ_FOR_EACH_I(RPC_ENUM_COCLASSES, RPC_DECLARE_COCLASS, coclasses)


#ifndef __RPCLIB_PROXY_H__

#define RPC_LIBRARY(name, id, ifaces, coclasses) RPC_LIBRARY_DECLARE_IFS(name, id, ifaces, coclasses)
#define RPC_DECLARE_IFS_PARAMETERS_SPECIALIZATION(ifs_type)

#endif

#define RPC_BEGIN_FACTORY(name) \
struct name\
{\
	static void *CreateObject(int nId)\
	{\
		void *pObject = NULL;\
		switch(nId)\
		{

#define RPC_OBJECT_ENTRY(obj, ifs)\
			case RPC_ID(obj): pObject = static_cast<ifs *>(new obj()); break;

#define RPC_STATIC_OBJECT_ENTRY(obj, ifs, val)\
			case RPC_ID(obj): pObject = static_cast<ifs *>(&val); break;


#define RPC_END_FACTORY() \
		}\
		return pObject;\
	}\
};


#endif

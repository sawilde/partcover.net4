//	tpes.h:		rpclib types definition
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__RPCLIB_TYPES_H__
#define __RPCLIB_TYPES_H__

namespace rpclib
{

typedef ptrdiff_t	Token;
typedef int			RPCRESULT;
typedef Token RPCIID;
typedef RPCIID RPCCLSID;

// pass by value
typedef RPCIID RPCREFIID;
typedef RPCCLSID RPCREFCLSID;


#define RPCSUCCEEDED(hr) ((rpclib::RPCRESULT)(hr) >= 0)
#define RPCFAILED(hr) ((rpclib::RPCRESULT)(hr) < 0)

#define	RPCRESULT_TYPEDEF(val) ((rpclib::RPCRESULT)val)
//#define RPCRESULT_FROM_WIN32(x) ((x <= 0) ? (RPCRESULT)x : (RPCRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000))

#define	RPC_ERROR_BASE			0x80000000
#undef  RPC_S_OK
#define	RPC_S_OK				RPCRESULT_TYPEDEF(0)
#define RPC_S_FALSE				RPCRESULT_TYPEDEF(1)

#define	RPC_E_NETWORK_ERROR		RPCRESULT_TYPEDEF(RPC_ERROR_BASE + 1)
#define	RPC_E_FAIL				RPCRESULT_TYPEDEF(RPC_ERROR_BASE + 8)

#ifndef RPC_METHOD_CALL_TYPE
#define RPC_METHOD_CALL_TYPE
#endif

template <class T>
struct std_coclass: public T
{
//  this make this template usefull not only for concrete "leaf" classes
	virtual ~std_coclass()
	{
	}
	virtual RPCRESULT RPC_METHOD_CALL_TYPE Release()
	{
		delete this;
		return RPC_S_OK;
	}
};
//  rpclib smart ptr classes

template <class I>
struct CRpcPtr
{
	CRpcPtr():m_p(NULL){}
	CRpcPtr(I *p): m_p(p){}
	~CRpcPtr()
	{
		if (m_p)
			m_p->Release();
	}
	RPCRESULT Release()
	{
		RPCRESULT  nRet = m_p->Release();
		m_p = NULL;
		return nRet;
	}
	void Attach(I *)
	{
		assert(m_p == NULL);
		m_p = p;
	}
	operator I* ()
	{
		assert (m_p);
		return m_p;
	}
	I* Detach()
	{
		assert(m_p);
		I *p = m_p;
		m_p = NULL;
		return p;
	}
	I * operator -> ()
	{
		return m_p;
	}
	I ** operator &()
	{
		return &m_p;
	}
protected:
	CRpcPtr(CRpcPtr<I> &);
	I * operator = (I *);
	I * operator = (const CRpcPtr<I> &);
private:
	I *m_p;
};

template<class T>
struct rpc_guid;

template <class C, class I>	
struct RpcCoClassReg;


} // namespace rpclib


#define RPC_TYPE_REG(name) rpclib::rpc_guid<name>()
#define RPC_ID(name) static_cast<rpclib::RPCIID>(rpclib::rpc_guid<name>::ID)

#define RPC_DECLARE_GUID(name, id)\
namespace rpclib\
{\
template<>\
struct rpc_guid<name>\
{\
	enum {ID = id};\
	typedef name type;\
};\
}

#define RPC_REGISTER_COCLASS(c, ifs)\
namespace rpclib\
{\
template <>	\
struct RpcCoClassReg<c, ifs>\
{\
	typedef c	coclass;\
	typedef ifs iface;\
};\
}

#define RPC_VERIFY_COCLASS_REG(c, ifs) struct rpc_verify_coclass_reg{typedef typename rpclib::template RpcCoClassReg<c, ifs>::coclass policy;}


#endif



/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Tue Jul 04 12:09:13 2006
 */
/* Compiler settings for PartCover.CorDriver.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __PartCover2ECorDriver_h__
#define __PartCover2ECorDriver_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPartCoverConnector_FWD_DEFINED__
#define __IPartCoverConnector_FWD_DEFINED__
typedef interface IPartCoverConnector IPartCoverConnector;
#endif 	/* __IPartCoverConnector_FWD_DEFINED__ */


#ifndef __CorProfiler_FWD_DEFINED__
#define __CorProfiler_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorProfiler CorProfiler;
#else
typedef struct CorProfiler CorProfiler;
#endif /* __cplusplus */

#endif 	/* __CorProfiler_FWD_DEFINED__ */


#ifndef __PartCoverConnector_FWD_DEFINED__
#define __PartCoverConnector_FWD_DEFINED__

#ifdef __cplusplus
typedef class PartCoverConnector PartCoverConnector;
#else
typedef struct PartCoverConnector PartCoverConnector;
#endif /* __cplusplus */

#endif 	/* __PartCoverConnector_FWD_DEFINED__ */


/* header files for imported files */
#include "corsym.h"
#include "corprof.h"
#include "docobj.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IPartCoverConnector_INTERFACE_DEFINED__
#define __IPartCoverConnector_INTERFACE_DEFINED__

/* interface IPartCoverConnector */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IPartCoverConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AF8D2B08-1DDF-4AE5-986C-09EDD8326045")
    IPartCoverConnector : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartTarget( 
            /* [in] */ BSTR targetPath,
            /* [in] */ BSTR targetWorkingDir,
            /* [in] */ BSTR targetArguments) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLogging( 
            /* [in] */ VARIANT_BOOL enable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableProfilerMode( 
            /* [in] */ enum ProfilerMode mode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginWork( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndWork( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReceiveResult( 
            /* [in] */ IStream *result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndTarget( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPartCoverConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPartCoverConnector * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPartCoverConnector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPartCoverConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartTarget )( 
            IPartCoverConnector * This,
            /* [in] */ BSTR targetPath,
            /* [in] */ BSTR targetWorkingDir,
            /* [in] */ BSTR targetArguments);
        
        HRESULT ( STDMETHODCALLTYPE *SetLogging )( 
            IPartCoverConnector * This,
            /* [in] */ VARIANT_BOOL enable);
        
        HRESULT ( STDMETHODCALLTYPE *EnableProfilerMode )( 
            IPartCoverConnector * This,
            /* [in] */ enum ProfilerMode mode);
        
        HRESULT ( STDMETHODCALLTYPE *BeginWork )( 
            IPartCoverConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndWork )( 
            IPartCoverConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReceiveResult )( 
            IPartCoverConnector * This,
            /* [in] */ IStream *result);
        
        HRESULT ( STDMETHODCALLTYPE *EndTarget )( 
            IPartCoverConnector * This);
        
        END_INTERFACE
    } IPartCoverConnectorVtbl;

    interface IPartCoverConnector
    {
        CONST_VTBL struct IPartCoverConnectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPartCoverConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPartCoverConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPartCoverConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPartCoverConnector_StartTarget(This,targetPath,targetWorkingDir,targetArguments)	\
    (This)->lpVtbl -> StartTarget(This,targetPath,targetWorkingDir,targetArguments)

#define IPartCoverConnector_SetLogging(This,enable)	\
    (This)->lpVtbl -> SetLogging(This,enable)

#define IPartCoverConnector_EnableProfilerMode(This,mode)	\
    (This)->lpVtbl -> EnableProfilerMode(This,mode)

#define IPartCoverConnector_BeginWork(This)	\
    (This)->lpVtbl -> BeginWork(This)

#define IPartCoverConnector_EndWork(This)	\
    (This)->lpVtbl -> EndWork(This)

#define IPartCoverConnector_ReceiveResult(This,result)	\
    (This)->lpVtbl -> ReceiveResult(This,result)

#define IPartCoverConnector_EndTarget(This)	\
    (This)->lpVtbl -> EndTarget(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPartCoverConnector_StartTarget_Proxy( 
    IPartCoverConnector * This,
    /* [in] */ BSTR targetPath,
    /* [in] */ BSTR targetWorkingDir,
    /* [in] */ BSTR targetArguments);


void __RPC_STUB IPartCoverConnector_StartTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPartCoverConnector_SetLogging_Proxy( 
    IPartCoverConnector * This,
    /* [in] */ VARIANT_BOOL enable);


void __RPC_STUB IPartCoverConnector_SetLogging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPartCoverConnector_EnableProfilerMode_Proxy( 
    IPartCoverConnector * This,
    /* [in] */ enum ProfilerMode mode);


void __RPC_STUB IPartCoverConnector_EnableProfilerMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPartCoverConnector_BeginWork_Proxy( 
    IPartCoverConnector * This);


void __RPC_STUB IPartCoverConnector_BeginWork_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPartCoverConnector_EndWork_Proxy( 
    IPartCoverConnector * This);


void __RPC_STUB IPartCoverConnector_EndWork_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPartCoverConnector_ReceiveResult_Proxy( 
    IPartCoverConnector * This,
    /* [in] */ IStream *result);


void __RPC_STUB IPartCoverConnector_ReceiveResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPartCoverConnector_EndTarget_Proxy( 
    IPartCoverConnector * This);


void __RPC_STUB IPartCoverConnector_EndTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPartCoverConnector_INTERFACE_DEFINED__ */



#ifndef __PartCover_LIBRARY_DEFINED__
#define __PartCover_LIBRARY_DEFINED__

/* library PartCover */
/* [helpstring][uuid][version] */ 

/* [custom][helpstring][uuid] */ 
enum  DECLSPEC_UUID("9BC23D20-04DE-4ee7-AB24-1E890C741F78") ProfilerMode
    {	COUNT_COVERAGE	= 1
    } ;

EXTERN_C const IID LIBID_PartCover;

EXTERN_C const CLSID CLSID_CorProfiler;

#ifdef __cplusplus

class DECLSPEC_UUID("89FC4679-2C1E-439d-A9E1-56B817C4C656")
CorProfiler;
#endif

EXTERN_C const CLSID CLSID_PartCoverConnector;

#ifdef __cplusplus

class DECLSPEC_UUID("15225ABF-81EE-4351-B336-AB38E5B95FD7")
PartCoverConnector;
#endif
#endif /* __PartCover_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



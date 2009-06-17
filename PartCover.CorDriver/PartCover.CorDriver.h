

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Wed Jun 17 20:30:12 2009
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


#ifndef __PartCover2ECorDriver_h__
#define __PartCover2ECorDriver_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IReportReceiver_FWD_DEFINED__
#define __IReportReceiver_FWD_DEFINED__
typedef interface IReportReceiver IReportReceiver;
#endif 	/* __IReportReceiver_FWD_DEFINED__ */


#ifndef __IConnectorActionCallback_FWD_DEFINED__
#define __IConnectorActionCallback_FWD_DEFINED__
typedef interface IConnectorActionCallback IConnectorActionCallback;
#endif 	/* __IConnectorActionCallback_FWD_DEFINED__ */


#ifndef __IPartCoverConnector2_FWD_DEFINED__
#define __IPartCoverConnector2_FWD_DEFINED__
typedef interface IPartCoverConnector2 IPartCoverConnector2;
#endif 	/* __IPartCoverConnector2_FWD_DEFINED__ */


#ifndef __CorProfiler_FWD_DEFINED__
#define __CorProfiler_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorProfiler CorProfiler;
#else
typedef struct CorProfiler CorProfiler;
#endif /* __cplusplus */

#endif 	/* __CorProfiler_FWD_DEFINED__ */


#ifndef __PartCoverConnector2_FWD_DEFINED__
#define __PartCoverConnector2_FWD_DEFINED__

#ifdef __cplusplus
typedef class PartCoverConnector2 PartCoverConnector2;
#else
typedef struct PartCoverConnector2 PartCoverConnector2;
#endif /* __cplusplus */

#endif 	/* __PartCoverConnector2_FWD_DEFINED__ */


/* header files for imported files */
#include "corsym.h"
#include "corprof.h"
#include "docobj.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_PartCover2ECorDriver_0000_0000 */
/* [local] */ 

typedef /* [custom][helpstring][uuid] */  DECLSPEC_UUID("040790E6-D4A1-4579-A4B9-BF9B31E8F9E8") struct BLOCK_DATA
    {
    INT position;
    INT blockLen;
    INT visitCount;
    INT fileId;
    INT startLine;
    INT startColumn;
    INT endLine;
    INT endColumn;
    } 	BLOCK_DATA;

typedef /* [custom][helpstring][uuid] */  DECLSPEC_UUID("37DE0E5B-CA8B-4a89-9071-10417AA4BCF4") struct MEMORY_COUNTERS
    {
    DWORD PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    } 	MEMORY_COUNTERS;



extern RPC_IF_HANDLE __MIDL_itf_PartCover2ECorDriver_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PartCover2ECorDriver_0000_0000_v0_0_s_ifspec;


#ifndef __PartCover_LIBRARY_DEFINED__
#define __PartCover_LIBRARY_DEFINED__

/* library PartCover */
/* [helpstring][uuid][version] */ 

/* [custom][helpstring][uuid] */ 
enum  DECLSPEC_UUID("9BC23D20-04DE-4ee7-AB24-1E890C741F78") ProfilerMode
    {	COUNT_COVERAGE	= 1,
	COVERAGE_USE_CLASS_LEVEL	= 4,
	COVERAGE_USE_ASSEMBLY_LEVEL	= 8
    } ;

EXTERN_C const IID LIBID_PartCover;

#ifndef __IReportReceiver_INTERFACE_DEFINED__
#define __IReportReceiver_INTERFACE_DEFINED__

/* interface IReportReceiver */
/* [custom][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IReportReceiver;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4BAD004E-1EF9-43d2-8D3A-095963E324EF")
    IReportReceiver : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterFile( 
            /* [in] */ INT fileId,
            /* [in] */ BSTR fileUrl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterSkippedItem( 
            /* [in] */ BSTR assemblyName,
            /* [in] */ BSTR typedefName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnterAssembly( 
            /* [in] */ INT domain,
            /* [in] */ BSTR domainName,
            /* [in] */ BSTR assemblyName,
            /* [in] */ BSTR moduleName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnterTypedef( 
            /* [in] */ BSTR typedefName,
            /* [in] */ DWORD flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnterMethod( 
            /* [in] */ BSTR methodName,
            /* [in] */ BSTR methodSig,
            /* [in] */ INT bodySize,
            /* [in] */ DWORD flags,
            /* [in] */ DWORD implFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddCoverageBlock( 
            /* [in] */ BLOCK_DATA blockData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveMethod( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveTypedef( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LeaveAssembly( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IReportReceiverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IReportReceiver * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IReportReceiver * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IReportReceiver * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterFile )( 
            IReportReceiver * This,
            /* [in] */ INT fileId,
            /* [in] */ BSTR fileUrl);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterSkippedItem )( 
            IReportReceiver * This,
            /* [in] */ BSTR assemblyName,
            /* [in] */ BSTR typedefName);
        
        HRESULT ( STDMETHODCALLTYPE *EnterAssembly )( 
            IReportReceiver * This,
            /* [in] */ INT domain,
            /* [in] */ BSTR domainName,
            /* [in] */ BSTR assemblyName,
            /* [in] */ BSTR moduleName);
        
        HRESULT ( STDMETHODCALLTYPE *EnterTypedef )( 
            IReportReceiver * This,
            /* [in] */ BSTR typedefName,
            /* [in] */ DWORD flags);
        
        HRESULT ( STDMETHODCALLTYPE *EnterMethod )( 
            IReportReceiver * This,
            /* [in] */ BSTR methodName,
            /* [in] */ BSTR methodSig,
            /* [in] */ INT bodySize,
            /* [in] */ DWORD flags,
            /* [in] */ DWORD implFlags);
        
        HRESULT ( STDMETHODCALLTYPE *AddCoverageBlock )( 
            IReportReceiver * This,
            /* [in] */ BLOCK_DATA blockData);
        
        HRESULT ( STDMETHODCALLTYPE *LeaveMethod )( 
            IReportReceiver * This);
        
        HRESULT ( STDMETHODCALLTYPE *LeaveTypedef )( 
            IReportReceiver * This);
        
        HRESULT ( STDMETHODCALLTYPE *LeaveAssembly )( 
            IReportReceiver * This);
        
        END_INTERFACE
    } IReportReceiverVtbl;

    interface IReportReceiver
    {
        CONST_VTBL struct IReportReceiverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReportReceiver_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IReportReceiver_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IReportReceiver_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IReportReceiver_RegisterFile(This,fileId,fileUrl)	\
    ( (This)->lpVtbl -> RegisterFile(This,fileId,fileUrl) ) 

#define IReportReceiver_RegisterSkippedItem(This,assemblyName,typedefName)	\
    ( (This)->lpVtbl -> RegisterSkippedItem(This,assemblyName,typedefName) ) 

#define IReportReceiver_EnterAssembly(This,domain,domainName,assemblyName,moduleName)	\
    ( (This)->lpVtbl -> EnterAssembly(This,domain,domainName,assemblyName,moduleName) ) 

#define IReportReceiver_EnterTypedef(This,typedefName,flags)	\
    ( (This)->lpVtbl -> EnterTypedef(This,typedefName,flags) ) 

#define IReportReceiver_EnterMethod(This,methodName,methodSig,bodySize,flags,implFlags)	\
    ( (This)->lpVtbl -> EnterMethod(This,methodName,methodSig,bodySize,flags,implFlags) ) 

#define IReportReceiver_AddCoverageBlock(This,blockData)	\
    ( (This)->lpVtbl -> AddCoverageBlock(This,blockData) ) 

#define IReportReceiver_LeaveMethod(This)	\
    ( (This)->lpVtbl -> LeaveMethod(This) ) 

#define IReportReceiver_LeaveTypedef(This)	\
    ( (This)->lpVtbl -> LeaveTypedef(This) ) 

#define IReportReceiver_LeaveAssembly(This)	\
    ( (This)->lpVtbl -> LeaveAssembly(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IReportReceiver_INTERFACE_DEFINED__ */


#ifndef __IConnectorActionCallback_INTERFACE_DEFINED__
#define __IConnectorActionCallback_INTERFACE_DEFINED__

/* interface IConnectorActionCallback */
/* [custom][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IConnectorActionCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64845E73-9471-401d-AEB8-B6B24CF0E894")
    IConnectorActionCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetConnected( 
            /* [in] */ VARIANT_BOOL connected) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowTargetMemory( 
            /* [in] */ MEMORY_COUNTERS counters) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MethodsReceiveBegin( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MethodsReceiveStatus( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MethodsReceiveEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveBegin( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveStatus( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveFilesBegin( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveFilesCount( 
            /* [in] */ SIZE_T fileCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveFilesStat( 
            /* [in] */ SIZE_T index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveFilesEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveSkippedBegin( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveSkippedCount( 
            /* [in] */ SIZE_T itemCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveSkippedStat( 
            /* [in] */ SIZE_T index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveSkippedEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveCountersBegin( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveCountersAsmCount( 
            /* [in] */ SIZE_T asmCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveCountersAsm( 
            /* [in] */ BSTR name,
            /* [in] */ BSTR mod,
            /* [in] */ SIZE_T typeDefCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstrumentDataReceiveCountersEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenMessagePipe( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TargetSetEnvironmentVars( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TargetCreateProcess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TargetWaitDriver( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TargetRequestShutdown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DriverConnected( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DriverSendRules( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DriverWaitEoIConfirm( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FunctionsReceiveBegin( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FunctionsCount( 
            /* [in] */ SIZE_T count) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FunctionsReceiveStat( 
            /* [in] */ SIZE_T index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FunctionsReceiveEnd( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogMessage( 
            /* [in] */ INT threadId,
            /* [in] */ LONG tick,
            /* [in] */ BSTR message) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IConnectorActionCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConnectorActionCallback * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConnectorActionCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetConnected )( 
            IConnectorActionCallback * This,
            /* [in] */ VARIANT_BOOL connected);
        
        HRESULT ( STDMETHODCALLTYPE *ShowTargetMemory )( 
            IConnectorActionCallback * This,
            /* [in] */ MEMORY_COUNTERS counters);
        
        HRESULT ( STDMETHODCALLTYPE *MethodsReceiveBegin )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *MethodsReceiveStatus )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *MethodsReceiveEnd )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveBegin )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveStatus )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveEnd )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveFilesBegin )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveFilesCount )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T fileCount);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveFilesStat )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T index);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveFilesEnd )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveSkippedBegin )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveSkippedCount )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T itemCount);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveSkippedStat )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T index);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveSkippedEnd )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveCountersBegin )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveCountersAsmCount )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T asmCount);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveCountersAsm )( 
            IConnectorActionCallback * This,
            /* [in] */ BSTR name,
            /* [in] */ BSTR mod,
            /* [in] */ SIZE_T typeDefCount);
        
        HRESULT ( STDMETHODCALLTYPE *InstrumentDataReceiveCountersEnd )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *OpenMessagePipe )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *TargetSetEnvironmentVars )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *TargetCreateProcess )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *TargetWaitDriver )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *TargetRequestShutdown )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *DriverConnected )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *DriverSendRules )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *DriverWaitEoIConfirm )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *FunctionsReceiveBegin )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *FunctionsCount )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T count);
        
        HRESULT ( STDMETHODCALLTYPE *FunctionsReceiveStat )( 
            IConnectorActionCallback * This,
            /* [in] */ SIZE_T index);
        
        HRESULT ( STDMETHODCALLTYPE *FunctionsReceiveEnd )( 
            IConnectorActionCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *LogMessage )( 
            IConnectorActionCallback * This,
            /* [in] */ INT threadId,
            /* [in] */ LONG tick,
            /* [in] */ BSTR message);
        
        END_INTERFACE
    } IConnectorActionCallbackVtbl;

    interface IConnectorActionCallback
    {
        CONST_VTBL struct IConnectorActionCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnectorActionCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IConnectorActionCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IConnectorActionCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IConnectorActionCallback_SetConnected(This,connected)	\
    ( (This)->lpVtbl -> SetConnected(This,connected) ) 

#define IConnectorActionCallback_ShowTargetMemory(This,counters)	\
    ( (This)->lpVtbl -> ShowTargetMemory(This,counters) ) 

#define IConnectorActionCallback_MethodsReceiveBegin(This)	\
    ( (This)->lpVtbl -> MethodsReceiveBegin(This) ) 

#define IConnectorActionCallback_MethodsReceiveStatus(This)	\
    ( (This)->lpVtbl -> MethodsReceiveStatus(This) ) 

#define IConnectorActionCallback_MethodsReceiveEnd(This)	\
    ( (This)->lpVtbl -> MethodsReceiveEnd(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveBegin(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveBegin(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveStatus(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveStatus(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveEnd(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveEnd(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveFilesBegin(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveFilesBegin(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveFilesCount(This,fileCount)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveFilesCount(This,fileCount) ) 

#define IConnectorActionCallback_InstrumentDataReceiveFilesStat(This,index)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveFilesStat(This,index) ) 

#define IConnectorActionCallback_InstrumentDataReceiveFilesEnd(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveFilesEnd(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveSkippedBegin(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveSkippedBegin(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveSkippedCount(This,itemCount)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveSkippedCount(This,itemCount) ) 

#define IConnectorActionCallback_InstrumentDataReceiveSkippedStat(This,index)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveSkippedStat(This,index) ) 

#define IConnectorActionCallback_InstrumentDataReceiveSkippedEnd(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveSkippedEnd(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveCountersBegin(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveCountersBegin(This) ) 

#define IConnectorActionCallback_InstrumentDataReceiveCountersAsmCount(This,asmCount)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveCountersAsmCount(This,asmCount) ) 

#define IConnectorActionCallback_InstrumentDataReceiveCountersAsm(This,name,mod,typeDefCount)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveCountersAsm(This,name,mod,typeDefCount) ) 

#define IConnectorActionCallback_InstrumentDataReceiveCountersEnd(This)	\
    ( (This)->lpVtbl -> InstrumentDataReceiveCountersEnd(This) ) 

#define IConnectorActionCallback_OpenMessagePipe(This)	\
    ( (This)->lpVtbl -> OpenMessagePipe(This) ) 

#define IConnectorActionCallback_TargetSetEnvironmentVars(This)	\
    ( (This)->lpVtbl -> TargetSetEnvironmentVars(This) ) 

#define IConnectorActionCallback_TargetCreateProcess(This)	\
    ( (This)->lpVtbl -> TargetCreateProcess(This) ) 

#define IConnectorActionCallback_TargetWaitDriver(This)	\
    ( (This)->lpVtbl -> TargetWaitDriver(This) ) 

#define IConnectorActionCallback_TargetRequestShutdown(This)	\
    ( (This)->lpVtbl -> TargetRequestShutdown(This) ) 

#define IConnectorActionCallback_DriverConnected(This)	\
    ( (This)->lpVtbl -> DriverConnected(This) ) 

#define IConnectorActionCallback_DriverSendRules(This)	\
    ( (This)->lpVtbl -> DriverSendRules(This) ) 

#define IConnectorActionCallback_DriverWaitEoIConfirm(This)	\
    ( (This)->lpVtbl -> DriverWaitEoIConfirm(This) ) 

#define IConnectorActionCallback_FunctionsReceiveBegin(This)	\
    ( (This)->lpVtbl -> FunctionsReceiveBegin(This) ) 

#define IConnectorActionCallback_FunctionsCount(This,count)	\
    ( (This)->lpVtbl -> FunctionsCount(This,count) ) 

#define IConnectorActionCallback_FunctionsReceiveStat(This,index)	\
    ( (This)->lpVtbl -> FunctionsReceiveStat(This,index) ) 

#define IConnectorActionCallback_FunctionsReceiveEnd(This)	\
    ( (This)->lpVtbl -> FunctionsReceiveEnd(This) ) 

#define IConnectorActionCallback_LogMessage(This,threadId,tick,message)	\
    ( (This)->lpVtbl -> LogMessage(This,threadId,tick,message) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IConnectorActionCallback_INTERFACE_DEFINED__ */


#ifndef __IPartCoverConnector2_INTERFACE_DEFINED__
#define __IPartCoverConnector2_INTERFACE_DEFINED__

/* interface IPartCoverConnector2 */
/* [custom][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IPartCoverConnector2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64D5D652-8BF4-4E16-B192-80B6CE9147AD")
    IPartCoverConnector2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartTarget( 
            /* [in] */ BSTR targetPath,
            /* [in] */ BSTR targetWorkingDir,
            /* [in] */ BSTR targetArguments,
            /* [in] */ VARIANT_BOOL redirectOutput) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_LoggingLevel( 
            /* [in] */ INT logLevel) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_FileLoggingEnable( 
            /* [in] */ VARIANT_BOOL exitCode) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_PipeLoggingEnable( 
            /* [in] */ VARIANT_BOOL exitCode) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_StatusCallback( 
            /* [in] */ IConnectorActionCallback *callback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableOption( 
            /* [in] */ enum ProfilerMode mode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitForResults( 
            /* [in] */ VARIANT_BOOL delayClose) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReport( 
            /* [in] */ IReportReceiver *receiver) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IncludeItem( 
            /* [in] */ BSTR item) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExcludeItem( 
            /* [in] */ BSTR item) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_HasTargetExitCode( 
            /* [retval][out] */ VARIANT_BOOL *exitCode) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_TargetExitCode( 
            /* [retval][out] */ INT *exitCode) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_LogFilePath( 
            /* [retval][out] */ BSTR *logFilePath) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ProcessId( 
            /* [retval][out] */ INT *pid) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_StatusCallback( 
            /* [retval][out] */ IConnectorActionCallback **callback) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPartCoverConnector2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPartCoverConnector2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPartCoverConnector2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPartCoverConnector2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartTarget )( 
            IPartCoverConnector2 * This,
            /* [in] */ BSTR targetPath,
            /* [in] */ BSTR targetWorkingDir,
            /* [in] */ BSTR targetArguments,
            /* [in] */ VARIANT_BOOL redirectOutput);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_LoggingLevel )( 
            IPartCoverConnector2 * This,
            /* [in] */ INT logLevel);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_FileLoggingEnable )( 
            IPartCoverConnector2 * This,
            /* [in] */ VARIANT_BOOL exitCode);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_PipeLoggingEnable )( 
            IPartCoverConnector2 * This,
            /* [in] */ VARIANT_BOOL exitCode);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_StatusCallback )( 
            IPartCoverConnector2 * This,
            /* [in] */ IConnectorActionCallback *callback);
        
        HRESULT ( STDMETHODCALLTYPE *EnableOption )( 
            IPartCoverConnector2 * This,
            /* [in] */ enum ProfilerMode mode);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForResults )( 
            IPartCoverConnector2 * This,
            /* [in] */ VARIANT_BOOL delayClose);
        
        HRESULT ( STDMETHODCALLTYPE *GetReport )( 
            IPartCoverConnector2 * This,
            /* [in] */ IReportReceiver *receiver);
        
        HRESULT ( STDMETHODCALLTYPE *IncludeItem )( 
            IPartCoverConnector2 * This,
            /* [in] */ BSTR item);
        
        HRESULT ( STDMETHODCALLTYPE *ExcludeItem )( 
            IPartCoverConnector2 * This,
            /* [in] */ BSTR item);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_HasTargetExitCode )( 
            IPartCoverConnector2 * This,
            /* [retval][out] */ VARIANT_BOOL *exitCode);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_TargetExitCode )( 
            IPartCoverConnector2 * This,
            /* [retval][out] */ INT *exitCode);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_LogFilePath )( 
            IPartCoverConnector2 * This,
            /* [retval][out] */ BSTR *logFilePath);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_ProcessId )( 
            IPartCoverConnector2 * This,
            /* [retval][out] */ INT *pid);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_StatusCallback )( 
            IPartCoverConnector2 * This,
            /* [retval][out] */ IConnectorActionCallback **callback);
        
        END_INTERFACE
    } IPartCoverConnector2Vtbl;

    interface IPartCoverConnector2
    {
        CONST_VTBL struct IPartCoverConnector2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPartCoverConnector2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPartCoverConnector2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPartCoverConnector2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPartCoverConnector2_StartTarget(This,targetPath,targetWorkingDir,targetArguments,redirectOutput)	\
    ( (This)->lpVtbl -> StartTarget(This,targetPath,targetWorkingDir,targetArguments,redirectOutput) ) 

#define IPartCoverConnector2_put_LoggingLevel(This,logLevel)	\
    ( (This)->lpVtbl -> put_LoggingLevel(This,logLevel) ) 

#define IPartCoverConnector2_put_FileLoggingEnable(This,exitCode)	\
    ( (This)->lpVtbl -> put_FileLoggingEnable(This,exitCode) ) 

#define IPartCoverConnector2_put_PipeLoggingEnable(This,exitCode)	\
    ( (This)->lpVtbl -> put_PipeLoggingEnable(This,exitCode) ) 

#define IPartCoverConnector2_put_StatusCallback(This,callback)	\
    ( (This)->lpVtbl -> put_StatusCallback(This,callback) ) 

#define IPartCoverConnector2_EnableOption(This,mode)	\
    ( (This)->lpVtbl -> EnableOption(This,mode) ) 

#define IPartCoverConnector2_WaitForResults(This,delayClose)	\
    ( (This)->lpVtbl -> WaitForResults(This,delayClose) ) 

#define IPartCoverConnector2_GetReport(This,receiver)	\
    ( (This)->lpVtbl -> GetReport(This,receiver) ) 

#define IPartCoverConnector2_IncludeItem(This,item)	\
    ( (This)->lpVtbl -> IncludeItem(This,item) ) 

#define IPartCoverConnector2_ExcludeItem(This,item)	\
    ( (This)->lpVtbl -> ExcludeItem(This,item) ) 

#define IPartCoverConnector2_get_HasTargetExitCode(This,exitCode)	\
    ( (This)->lpVtbl -> get_HasTargetExitCode(This,exitCode) ) 

#define IPartCoverConnector2_get_TargetExitCode(This,exitCode)	\
    ( (This)->lpVtbl -> get_TargetExitCode(This,exitCode) ) 

#define IPartCoverConnector2_get_LogFilePath(This,logFilePath)	\
    ( (This)->lpVtbl -> get_LogFilePath(This,logFilePath) ) 

#define IPartCoverConnector2_get_ProcessId(This,pid)	\
    ( (This)->lpVtbl -> get_ProcessId(This,pid) ) 

#define IPartCoverConnector2_get_StatusCallback(This,callback)	\
    ( (This)->lpVtbl -> get_StatusCallback(This,callback) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPartCoverConnector2_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CorProfiler;

#ifdef __cplusplus

class DECLSPEC_UUID("717FF691-2ADF-4AC0-985F-1DD3C42FDF90")
CorProfiler;
#endif

EXTERN_C const CLSID CLSID_PartCoverConnector2;

#ifdef __cplusplus

class DECLSPEC_UUID("FB20430E-CDC9-45D7-8453-272268002E08")
PartCoverConnector2;
#endif
#endif /* __PartCover_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



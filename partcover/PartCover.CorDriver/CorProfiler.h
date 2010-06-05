#pragma once

[   
    coclass,
    uuid(DRIVER_CORPROFILER2_GUID),
    vi_progid(DRIVER_CORPROFILER_VI_PROGID),
    progid(DRIVER_CORPROFILER2_PROGID),
    threading(both),
    helpstring("CorDriver.CorProfiler2 Class"),
    version(DRIVER_CORPROFILER2_VER)
]
class CorProfiler 
	: public ICorProfilerCallback2
{
    static CorProfiler* m_currentInstance;

public:
    static void FinalizeInstance();

    HRESULT FinalConstruct() { m_currentInstance = this; return S_OK; }
    HRESULT FinalRelease() { m_currentInstance = 0; return S_OK; }

    CorProfiler();

private:

    FunctionMap m_functions;

    Rules m_rules;
    Instrumentator m_instrumentator;
    InstrumentResults m_instrumentResults;

    PartCoverMessageClient m_communication;
    CorProfilerOptions m_options;
    CComQIPtr<ICorProfilerInfo> m_profilerInfo;
    CComQIPtr<ISymUnmanagedBinder2> m_binder;

public:

    /************************************************************************/
    /* ICorProfilerCallback methods                                         */
    /************************************************************************/
public:
    STDMETHOD( Initialize( /* [in] */ IUnknown *pICorProfilerInfoUnk) );

    STDMETHOD( Shutdown( void) );

    STDMETHOD( AppDomainCreationStarted( 
        /* [in] */ AppDomainID appDomainId) )
    { return S_OK; }

    STDMETHOD( AppDomainCreationFinished( 
        /* [in] */ AppDomainID appDomainId,
        /* [in] */ HRESULT hrStatus) );

    STDMETHOD( AppDomainShutdownStarted( 
        /* [in] */ AppDomainID appDomainId) )
    { return S_OK; }

    STDMETHOD( AppDomainShutdownFinished( 
        /* [in] */ AppDomainID appDomainId,
        /* [in] */ HRESULT hrStatus) );

    STDMETHOD( AssemblyLoadStarted( 
        /* [in] */ AssemblyID assemblyId) )
    { return S_OK; }

    STDMETHOD( AssemblyLoadFinished( 
        /* [in] */ AssemblyID assemblyId,
        /* [in] */ HRESULT hrStatus) )
    ;

    STDMETHOD( AssemblyUnloadStarted( 
        /* [in] */ AssemblyID assemblyId) )
    ;
    

    STDMETHOD( AssemblyUnloadFinished( 
        /* [in] */ AssemblyID assemblyId,
        /* [in] */ HRESULT hrStatus) )
    { return S_OK; }

    STDMETHOD( ModuleLoadStarted( 
        /* [in] */ ModuleID moduleId) )
    { return S_OK; }

    STDMETHOD( ModuleLoadFinished( 
        /* [in] */ ModuleID moduleId,
        /* [in] */ HRESULT hrStatus) )
    ;

    STDMETHOD( ModuleUnloadStarted( 
        /* [in] */ ModuleID moduleId) )
    ;

    STDMETHOD( ModuleUnloadFinished( 
        /* [in] */ ModuleID moduleId,
        /* [in] */ HRESULT hrStatus) )
    { return S_OK; }

    STDMETHOD( ModuleAttachedToAssembly( 
        /* [in] */ ModuleID moduleId,
        /* [in] */ AssemblyID AssemblyId) )
    ;

    STDMETHOD( ClassLoadStarted( 
        /* [in] */ ClassID classId) )
    { return S_OK; }

    STDMETHOD( ClassLoadFinished( 
        /* [in] */ ClassID classId,
        /* [in] */ HRESULT hrStatus) )
    ;

    STDMETHOD( ClassUnloadStarted( 
        /* [in] */ ClassID classId) )
    ;

    STDMETHOD( ClassUnloadFinished( 
        /* [in] */ ClassID classId,
        /* [in] */ HRESULT hrStatus) )
    { return S_OK; }

    STDMETHOD( FunctionUnloadStarted( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( JITCompilationStarted( 
        /* [in] */ FunctionID functionId,
        /* [in] */ BOOL fIsSafeToBlock) );

    STDMETHOD( JITCompilationFinished( 
        /* [in] */ FunctionID functionId,
        /* [in] */ HRESULT hrStatus,
        /* [in] */ BOOL fIsSafeToBlock) )
    { return S_OK; }

    STDMETHOD( JITCachedFunctionSearchStarted( 
        /* [in] */ FunctionID functionId,
        /* [out] */ BOOL *pbUseCachedFunction) )
    { return S_OK; }

    STDMETHOD( JITCachedFunctionSearchFinished( 
        /* [in] */ FunctionID functionId,
        /* [in] */ COR_PRF_JIT_CACHE result) )
    { return S_OK; }

    STDMETHOD( JITFunctionPitched( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( JITInlining( 
        /* [in] */ FunctionID callerId,
        /* [in] */ FunctionID calleeId,
        /* [out] */ BOOL *pfShouldInline) )
    { return S_OK; }

    STDMETHOD( ThreadCreated( 
        /* [in] */ ThreadID threadId) )
    { return S_OK; }

    STDMETHOD( ThreadDestroyed( 
        /* [in] */ ThreadID threadId) )
    { return S_OK; }

    STDMETHOD( ThreadAssignedToOSThread( 
        /* [in] */ ThreadID managedThreadId,
        /* [in] */ DWORD osThreadId) )
    { return S_OK; }

    STDMETHOD( RemotingClientInvocationStarted( void) )
    { return S_OK; }

    STDMETHOD( RemotingClientSendingMessage( 
        /* [in] */ GUID *pCookie,
        /* [in] */ BOOL fIsAsync) )
    { return S_OK; }

    STDMETHOD( RemotingClientReceivingReply( 
        /* [in] */ GUID *pCookie,
        /* [in] */ BOOL fIsAsync) )
    { return S_OK; }

    STDMETHOD( RemotingClientInvocationFinished( void) )
    { return S_OK; }

    STDMETHOD( RemotingServerReceivingMessage( 
        /* [in] */ GUID *pCookie,
        /* [in] */ BOOL fIsAsync) )
    { return S_OK; }

    STDMETHOD( RemotingServerInvocationStarted( void) )
    { return S_OK; }

    STDMETHOD( RemotingServerInvocationReturned( void) )
    { return S_OK; }

    STDMETHOD( RemotingServerSendingReply( 
        /* [in] */ GUID *pCookie,
        /* [in] */ BOOL fIsAsync) )
    { return S_OK; }

    STDMETHOD( UnmanagedToManagedTransition( 
        /* [in] */ FunctionID functionId,
        /* [in] */ COR_PRF_TRANSITION_REASON reason) )
    { return S_OK; }

    STDMETHOD( ManagedToUnmanagedTransition( 
        /* [in] */ FunctionID functionId,
        /* [in] */ COR_PRF_TRANSITION_REASON reason) )
    { return S_OK; }

    STDMETHOD( RuntimeSuspendStarted( 
        /* [in] */ COR_PRF_SUSPEND_REASON suspendReason) )
    { return S_OK; }

    STDMETHOD( RuntimeSuspendFinished( void) )
    { return S_OK; }

    STDMETHOD( RuntimeSuspendAborted( void) )
    { return S_OK; }

    STDMETHOD( RuntimeResumeStarted( void) )
    { return S_OK; }

    STDMETHOD( RuntimeResumeFinished( void) )
    { return S_OK; }

    STDMETHOD( RuntimeThreadSuspended( 
        /* [in] */ ThreadID threadId) )
    { return S_OK; }

    STDMETHOD( RuntimeThreadResumed( 
        /* [in] */ ThreadID threadId) )
    { return S_OK; }

    STDMETHOD( MovedReferences( 
        /* [in] */ ULONG cMovedObjectIDRanges,
        /* [size_is][in] */ ObjectID oldObjectIDRangeStart[  ],
        /* [size_is][in] */ ObjectID newObjectIDRangeStart[  ],
        /* [size_is][in] */ ULONG cObjectIDRangeLength[  ]) )
    { return S_OK; }

    STDMETHOD( ObjectAllocated( 
        /* [in] */ ObjectID objectId,
        /* [in] */ ClassID classId) )
    { return S_OK; }

    STDMETHOD( ObjectsAllocatedByClass( 
        /* [in] */ ULONG cClassCount,
        /* [size_is][in] */ ClassID classIds[  ],
        /* [size_is][in] */ ULONG cObjects[  ]) )
    { return S_OK; }

    STDMETHOD( ObjectReferences( 
        /* [in] */ ObjectID objectId,
        /* [in] */ ClassID classId,
        /* [in] */ ULONG cObjectRefs,
        /* [size_is][in] */ ObjectID objectRefIds[  ]) )
    { return S_OK; }

    STDMETHOD( RootReferences( 
        /* [in] */ ULONG cRootRefs,
        /* [size_is][in] */ ObjectID rootRefIds[  ]) )
    { return S_OK; }

    STDMETHOD( ExceptionThrown( 
        /* [in] */ ObjectID thrownObjectId) )
    { return S_OK; }

    STDMETHOD( ExceptionSearchFunctionEnter( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( ExceptionSearchFunctionLeave( void) )
    { return S_OK; }

    STDMETHOD( ExceptionSearchFilterEnter( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( ExceptionSearchFilterLeave( void) )
    { return S_OK; }

    STDMETHOD( ExceptionSearchCatcherFound( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( ExceptionOSHandlerEnter( 
        /* [in] */ UINT_PTR __unused) )
    { return S_OK; }

    STDMETHOD( ExceptionOSHandlerLeave( 
        /* [in] */ UINT_PTR __unused) )
    { return S_OK; }

    STDMETHOD( ExceptionUnwindFunctionEnter( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( ExceptionUnwindFunctionLeave( void) )
    { return S_OK; }

    STDMETHOD( ExceptionUnwindFinallyEnter( 
        /* [in] */ FunctionID functionId) )
    { return S_OK; }

    STDMETHOD( ExceptionUnwindFinallyLeave( void) )
    { return S_OK; }

    STDMETHOD( ExceptionCatcherEnter( 
        /* [in] */ FunctionID functionId,
        /* [in] */ ObjectID objectId) )
    { return S_OK; }

    STDMETHOD( ExceptionCatcherLeave( void) )
    { return S_OK; }

    STDMETHOD( COMClassicVTableCreated( 
        /* [in] */ ClassID wrappedClassId,
        /* [in] */ REFGUID implementedIID,
        /* [in] */ void *pVTable,
        /* [in] */ ULONG cSlots) )
    { return S_OK; }

    STDMETHOD( COMClassicVTableDestroyed( 
        /* [in] */ ClassID wrappedClassId,
        /* [in] */ REFGUID implementedIID,
        /* [in] */ void *pVTable) )
    { return S_OK; }

    STDMETHOD( ExceptionCLRCatcherFound( void) )
    { return S_OK; }

    STDMETHOD( ExceptionCLRCatcherExecute( void) )
    { return S_OK; }

    STDMETHOD( ThreadNameChanged(
        /* [in] */ ThreadID threadId,
        /* [in] */ ULONG cchName,
        /* [in] */ WCHAR name[  ]) )
    { return S_OK; }

        
    STDMETHOD( GarbageCollectionStarted( 
        /* [in] */ int cGenerations,
        /* [length_is][size_is][in] */ BOOL generationCollected[  ],
        /* [in] */ COR_PRF_GC_REASON reason) )
    { return S_OK; }

    STDMETHOD( SurvivingReferences( 
        /* [in] */ ULONG cSurvivingObjectIDRanges,
        /* [size_is][in] */ ObjectID objectIDRangeStart[  ],
        /* [size_is][in] */ ULONG cObjectIDRangeLength[  ]) )
    { return S_OK; }
        
    STDMETHOD( GarbageCollectionFinished( void) )
    { return S_OK; }
        
    STDMETHOD( FinalizeableObjectQueued( 
        /* [in] */ DWORD finalizerFlags,
        /* [in] */ ObjectID objectID) )
    { return S_OK; }
        
    STDMETHOD( RootReferences2( 
        /* [in] */ ULONG cRootRefs,
        /* [size_is][in] */ ObjectID rootRefIds[  ],
        /* [size_is][in] */ COR_PRF_GC_ROOT_KIND rootKinds[  ],
        /* [size_is][in] */ COR_PRF_GC_ROOT_FLAGS rootFlags[  ],
        /* [size_is][in] */ UINT_PTR rootIds[  ]) )
    { return S_OK; }
        
    STDMETHOD( HandleCreated( 
        /* [in] */ GCHandleID handleId,
        /* [in] */ ObjectID initialObjectId) )
    { return S_OK; }
        
    STDMETHOD( HandleDestroyed( 
        /* [in] */ GCHandleID handleId) )
    { return S_OK; }
        
};

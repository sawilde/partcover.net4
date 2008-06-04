#pragma once

struct ResultType {
    
};

__interface IInstrumentedBlockWalker;

[
    object
    ,uuid(DRIVER_ICONNECTOR_GUID)
    ,helpstring("CorDriver.IPartCoverConnector interface")
    ,library_block
]
__interface IPartCoverConnector : IUnknown {
    HRESULT StartTarget([in] BSTR targetPath, [in] BSTR targetWorkingDir, [in] BSTR targetArguments, [in] VARIANT_BOOL redirectOutput);
    HRESULT SetVerbose([in] INT logLevel);
    HRESULT EnableOption([in] ProfilerMode mode);
    HRESULT WaitForResults([in] VARIANT_BOOL delayClose);
    HRESULT CloseTarget();
//    HRESULT WalkFunctions([in] IFunctionMapWalker* walker);
//    HRESULT WalkCallTree([in] ICallWalker* walker);

    HRESULT WalkInstrumentedResults([in] IInstrumentedBlockWalker* walker);

    HRESULT IncludeItem([in] BSTR item);
    HRESULT ExcludeItem([in] BSTR item);
};

[
    coclass
    ,uuid(DRIVER_CONNECTOR_GUID)
    ,vi_progid(DRIVER_CONNECTOR_VI_PROGID)
    ,progid(DRIVER_CONNECTOR_PROGID)
    ,threading(both)
    ,helpstring("CorDriver.PartCoverConnector Class")
    ,version(DRIVER_CONNECTOR_VER)
]
class PartCoverConnector : public IPartCoverConnector
{
    PROCESS_INFORMATION pi;

    int m_driverLogging;

    MessageCenter m_center;

    Rules m_rules;
    FunctionMap m_functions;
    CallGatherer m_callGatherer;
    CoverageGatherer m_coverageGatherer;
    InstrumentResults m_instrumentResults;

public:
    PartCoverConnector(void);
    ~PartCoverConnector(void);

    STDMETHOD(StartTarget)(BSTR targetPath, BSTR targetWorkingDir, BSTR targetArguments, VARIANT_BOOL redirectOutput);
    STDMETHOD(SetVerbose)(INT logLevel);
    STDMETHOD(EnableOption)(ProfilerMode mode);
    STDMETHOD(WaitForResults)(VARIANT_BOOL delayClose);
    STDMETHOD(CloseTarget)();

    STDMETHOD(WalkFunctions)(IFunctionMapWalker* walker);
    STDMETHOD(WalkCallTree)(ICallWalker* walker);
    STDMETHOD(WalkInstrumentedResults)(IInstrumentedBlockWalker* walker);

    STDMETHOD(IncludeItem)(BSTR item);
    STDMETHOD(ExcludeItem)(BSTR item);

};

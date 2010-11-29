#pragma once

interface IConnectorActionCallback;

class InstrumentResults
{
public:
    struct MethodBlock {
		MethodBlock() 
			: position(0), blockLength(0), visitCount(0)
		{}

        int position;
        int blockLength;
        long visitCount;
    };
    typedef std::vector<MethodBlock> MethodBlocks;

    struct MethodResult {
		MethodResult() 
			: flags(0), implFlags(0), bodySize(0), symbolFileId(-1), methodDef(0)
		{}

        String name;
        String sig;
		DWORD bodySize;

        DWORD flags;
        DWORD implFlags;

        BOOL symbolFileId;

		UINT methodDef;

        MethodBlocks blocks;
    };
    typedef std::vector<MethodResult> MethodResults;

    struct TypedefResult {
		TypedefResult() 
			: flags(0)
		{}

        String fullName;

        DWORD flags;

        MethodResults methods;
    };
    typedef std::vector<TypedefResult> TypedefResults;

    struct AssemblyResult {
        String name;
        String module;
        String domain;
		int domainIndex;
        TypedefResults types;
    };
    typedef std::vector<AssemblyResult> AssemblyResults;

	struct SkippedItem
	{
        String assemblyName;
		String typedefName;
	};
	typedef std::vector<SkippedItem> SkippedItems;

public:

    void GetReport(IReportReceiver& receiver);

    void Assign(AssemblyResults& results) { m_results.swap(results); }
	void Assign(SkippedItems& results) { m_skippedItems.swap(results); }

	void SetCallback(IConnectorActionCallback* callback) { m_callback = callback; }

	void Swap(InstrumentResults& other) 
	{
		m_results.swap(other.m_results);
		m_skippedItems.swap(other.m_skippedItems);
	}

private:

	IConnectorActionCallback* m_callback;

public:
    AssemblyResults m_results;
	SkippedItems m_skippedItems;
};

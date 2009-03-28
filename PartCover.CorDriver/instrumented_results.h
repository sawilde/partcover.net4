#pragma once

interface IConnectorActionCallback;

class InstrumentResults : public ITransferrable
{
public:
    struct MethodBlock {
		MethodBlock() 
			: haveSource(false)
			, sourceFileId(0), startLine(0), startColumn(0), endLine(0), endColumn(0)
			, position(0), blockLength(0), visitCount(0)
		{}

        bool haveSource;

        int sourceFileId;
        int startLine;
        int startColumn;
        int endLine;
        int endColumn;

        int position;
        int blockLength;
        long visitCount;
    };
    typedef std::vector<MethodBlock> MethodBlocks;

    struct MethodResult {
		MethodResult() 
			: flags(0), implFlags(0)
		{}

        String name;
        String sig;

        DWORD flags;
        DWORD implFlags;

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
        String moduleName;
        TypedefResults types;
    };
    typedef std::vector<AssemblyResult> AssemblyResults;

    struct FileItem {
		FileItem() : fileId(0) {}

        ULONG32      fileId;
        String fileUrl;
    };
    typedef std::vector<FileItem> FileItems;

	struct SkippedItem
	{
        String assemblyName;
		String typedefName;
	};
	typedef std::vector<SkippedItem> SkippedItems;

public:

    void GetReport(IReportReceiver& receiver);

    void Assign(AssemblyResults& results) { m_results.swap(results); }
    void Assign(FileItems& results);
	void Assign(SkippedItems& results) { m_skippedItems.swap(results); }

	MessageType getType() const { return Messages::C_InstrumentResults; }
	void visit(ITransferrableVisitor& visitor) { visitor.on(*this); }
    bool SendData(MessagePipe&);

    bool ReceiveData(MessagePipe&);
	bool ReceiveData(MessagePipe&, IConnectorActionCallback* callback);

	void SetCallback(IConnectorActionCallback* callback) { m_callback = callback; }
private:

	IConnectorActionCallback* m_callback;

    AssemblyResults m_results;
    FileItems m_fileTable;
	SkippedItems m_skippedItems;
};

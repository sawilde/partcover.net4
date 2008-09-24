#pragma once

interface IConnectorActionCallback;

class InstrumentResults : public ITransferrable
{
public:
    struct MethodBlock {
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
        String name;
        String sig;

        DWORD flags;
        DWORD implFlags;

        MethodBlocks blocks;
    };
    typedef std::vector<MethodResult> MethodResults;

    struct TypedefResult {
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
        ULONG32      fileId;
        String fileUrl;
    };
    typedef std::vector<FileItem> FileItems;

public:

    void WalkResults(IInstrumentedBlockWalker& walker);

    void Assign(AssemblyResults& results) { m_results.swap(results); }
    void Assign(FileItems& results);

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
};

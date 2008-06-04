#pragma once

class InstrumentResults : public IResultContainer
{
public:
    struct MethodBlock {
        bool haveSource;

        ULONG32 sourceFileId;
        ULONG32 startLine;
        ULONG32 startColumn;
        ULONG32 endLine;
        ULONG32 endColumn;

        DWORD position;
        ULONG blockLength;
        DWORD visitCount;
    };
    typedef std::vector<MethodBlock> MethodBlocks;

    struct MethodResult {
        std::wstring name;
        std::wstring sig;

        DWORD flags;
        DWORD implFlags;

        MethodBlocks blocks;
    };
    typedef std::vector<MethodResult> MethodResults;

    struct TypedefResult {
        std::wstring fullName;

        DWORD flags;

        MethodResults methods;
    };
    typedef std::vector<TypedefResult> TypedefResults;

    struct AssemblyResult {
        std::wstring name;
        std::wstring moduleName;
        TypedefResults types;
    };
    typedef std::vector<AssemblyResult> AssemblyResults;

    struct FileItem {
        ULONG32      fileId;
        std::wstring fileUrl;
    };
    typedef std::vector<FileItem> FileItems;

public:
    InstrumentResults(void);
    ~InstrumentResults(void);

    void SendResults(MessageCenter&);
    bool ReceiveResults(Message&);

    void WalkResults(IInstrumentedBlockWalker& walker);

    void Assign(AssemblyResults& results) { m_results.swap(results); }
    void Assign(FileItems& results);

private:

    AssemblyResults m_results;
    FileItems m_fileTable;

    void PushResults(AssemblyResults&, Message::byte_array& bytes);
    void PopResults(AssemblyResults&, Message::byte_array::iterator& source);
    void PushResults(FileItems&, Message::byte_array& bytes);
    void PopResults(FileItems&, Message::byte_array::iterator& source);
};

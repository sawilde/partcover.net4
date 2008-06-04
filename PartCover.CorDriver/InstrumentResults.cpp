#include "StdAfx.h"
#include "interface.h"
#include "MessageCenter.h"
#include "DriverLog.h"
#include "instrumentresults.h"

InstrumentResults::InstrumentResults(void)
{
}

InstrumentResults::~InstrumentResults(void)
{
}

void InstrumentResults::SendResults(MessageCenter& center) {
    Message message;
    message.code = eResult;
    Message::byte_array& value = message.value;

    value.push_back(eInstrumentatorResult);
    PushResults(m_results, value);
    PushResults(m_fileTable, value);

    ATLTRACE("InstrumentResults::SendResults - send eResult");
    center.SendOption(message);
}

bool InstrumentResults::ReceiveResults(Message& message) {
    typedef unsigned char* lpubyte;
    if (message.value.size() == 0 || message.value.front() != eInstrumentatorResult)
        return false;

    ATLTRACE("InstrumentResults::SendResults - receive result");
    Message::byte_array& value = message.value;
    Message::byte_array::iterator it = value.begin();

    size_t skipSize = 1;
    while(skipSize-- > 0) ++it;

    m_results.clear();
    PopResults(m_results, it);
    m_fileTable.clear();
    PopResults(m_fileTable, it);

    return true;
}

void InstrumentResults::PushResults(FileItems& files, Message::byte_array& bytes) {
    Message::push(bytes, files.size());
    for(FileItems::const_iterator asmIt = files.begin(); asmIt != files.end(); asmIt++) {
        const FileItem& item = *asmIt;
        Message::push(bytes, item.fileId);
        Message::push(bytes, item.fileUrl);
    }
}

void InstrumentResults::PopResults(FileItems& files, Message::byte_array::iterator& source) {
    size_t filesSize;

    source = Message::pop(source, &filesSize);
    files.resize(filesSize);
    for(size_t i = 0; i < filesSize; ++i) {
        FileItem& item = files[i];
        source = Message::pop(source, &item.fileId);
        source = Message::pop(source, &item.fileUrl);
    }
}

void InstrumentResults::PushResults(AssemblyResults& assemblies, Message::byte_array& bytes) {
    Message::push(bytes, assemblies.size());
    for(AssemblyResults::const_iterator asmIt = assemblies.begin(); asmIt != assemblies.end(); asmIt++) {
        const AssemblyResult& asmResult = *asmIt;
        Message::push(bytes, asmResult.name);
        Message::push(bytes, asmResult.moduleName);

        Message::push(bytes, asmResult.types.size());
        for(TypedefResults::const_iterator typeIt = asmResult.types.begin(); typeIt != asmResult.types.end(); typeIt++) {
            const TypedefResult& typedefResult = *typeIt;
            Message::push(bytes, typedefResult.fullName);
            Message::push(bytes, typedefResult.flags);

            Message::push(bytes, typedefResult.methods.size());
            for(MethodResults::const_iterator methodIt = typedefResult.methods.begin(); methodIt != typedefResult.methods.end(); methodIt++) {
                const MethodResult& methodResult = *methodIt;
                Message::push(bytes, methodResult.name);
                Message::push(bytes, methodResult.sig);
                Message::push(bytes, methodResult.flags);
                Message::push(bytes, methodResult.implFlags);

                Message::push(bytes, methodResult.blocks.size());
                for(MethodBlocks::const_iterator blockIt = methodResult.blocks.begin(); blockIt != methodResult.blocks.end(); blockIt++) {
                    const MethodBlock& block = *blockIt;

                    Message::push(bytes, block.position);
                    Message::push(bytes, block.blockLength);
                    Message::push(bytes, block.visitCount);
                    Message::push(bytes, block.haveSource);

                    if (block.haveSource) {
                        Message::push(bytes, block.sourceFileId);
                        Message::push(bytes, block.startLine);
                        Message::push(bytes, block.startColumn);
                        Message::push(bytes, block.endLine);
                        Message::push(bytes, block.endColumn);
                    }
                }
            }
        }
    }
}

void InstrumentResults::PopResults(AssemblyResults& assemblies, Message::byte_array::iterator& source) {
    size_t assemblySize;

    source = Message::pop(source, &assemblySize);
    assemblies.resize(assemblySize);
    for(size_t i = 0; i < assemblySize; ++i) {
        AssemblyResult& asmResult = assemblies[i];
        source = Message::pop(source, &asmResult.name);
        source = Message::pop(source, &asmResult.moduleName);

        size_t typesCount;
        source = Message::pop(source, &typesCount);
        asmResult.types.resize(typesCount);

        for(size_t j = 0; j < typesCount; ++j) {
            TypedefResult& typedefResult = asmResult.types[j];
            source = Message::pop(source, &typedefResult.fullName);
            source = Message::pop(source, &typedefResult.flags);

            size_t methodCount;
            source = Message::pop(source, &methodCount);
            typedefResult.methods.resize(methodCount);

            for(size_t m = 0; m < methodCount; ++m) {
                MethodResult& methodResult = typedefResult.methods[m];
                source = Message::pop(source, &methodResult.name);
                source = Message::pop(source, &methodResult.sig);
                source = Message::pop(source, &methodResult.flags);
                source = Message::pop(source, &methodResult.implFlags);

                size_t blockCount;
                source = Message::pop(source, &blockCount);
                methodResult.blocks.resize(blockCount);

                for(size_t b = 0; b < blockCount; ++b) {
                    MethodBlock& block = methodResult.blocks[b];

                    source = Message::pop(source, &block.position);
                    source = Message::pop(source, &block.blockLength);
                    source = Message::pop(source, &block.visitCount);
                    source = Message::pop(source, &block.haveSource);
                    if (block.haveSource) {
                        source = Message::pop(source, &block.sourceFileId);
                        source = Message::pop(source, &block.startLine);
                        source = Message::pop(source, &block.startColumn);
                        source = Message::pop(source, &block.endLine);
                        source = Message::pop(source, &block.endColumn);
                    }
                }
            }
        }
    }
}
void InstrumentResults::Assign(FileItems& results) { m_fileTable.swap(results); }

void InstrumentResults::WalkResults(IInstrumentedBlockWalker& walker) {
    if(FAILED(walker.BeginReport()))
        return;

    for(FileItems::const_iterator fileIt = m_fileTable.begin(); fileIt != m_fileTable.end(); fileIt++) {
        const FileItem& item = *fileIt;
        CComBSTR fileName(item.fileUrl.c_str());
        if(FAILED(walker.RegisterFile(item.fileId, fileName)))
            return;
    }

    for(AssemblyResults::const_iterator asmIt = m_results.begin(); asmIt != m_results.end(); asmIt++) {
        const AssemblyResult& asmResult = *asmIt;

        CComBSTR asmName(asmResult.name.c_str());
        CComBSTR moduleName(asmResult.moduleName.c_str());

        for(TypedefResults::const_iterator typeIt = asmResult.types.begin(); typeIt != asmResult.types.end(); typeIt++) {
            const TypedefResult& typedefResult = *typeIt;

            CComBSTR typedefName(typedefResult.fullName.c_str());
            if(FAILED(walker.EnterTypedef(asmName, typedefName, typedefResult.flags)))
                return;

            for(MethodResults::const_iterator methodIt = typedefResult.methods.begin(); methodIt != typedefResult.methods.end(); methodIt++) {
                const MethodResult& methodResult = *methodIt;

                CComBSTR methodName(methodResult.name.c_str());
                CComBSTR methodSig(methodResult.sig.c_str());
                if(FAILED(walker.EnterMethod(methodName, methodSig, methodResult.flags, methodResult.implFlags)))
                    return;

                for(MethodBlocks::const_iterator blockIt = methodResult.blocks.begin(); blockIt != methodResult.blocks.end(); blockIt++) {
                    const MethodBlock& block = *blockIt;

                    if(FAILED(walker.MethodBlock(block.position, block.blockLength, block.visitCount, 
                        block.haveSource ? block.sourceFileId : 0, 
                        block.startLine, block.startColumn, block.endLine, block.endColumn)))
                        return;
                }

                if(FAILED(walker.LeaveMethod()))
                    return;
            }

            if(FAILED(walker.LeaveTypedef()))
                return;
        }
    }

    if(FAILED(walker.EndReport()))
        return;
}
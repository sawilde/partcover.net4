#include "StdAfx.h"
#include "interface.h"
#include "message.h"
#include "message_pipe.h"
#include "logging.h"
#include "instrumented_results.h"

bool PushResults(InstrumentResults::FileItems& files, MessagePipe& pipe) {
	if(!pipe.write(files.size()))
		return false;
	bool result = true;
    for(InstrumentResults::FileItems::const_iterator it = files.begin(); result && it != files.end(); it++) {
		result = 
			pipe.write(it->fileId) && 
			pipe.write(it->fileUrl);
    }
	return result;
}

bool PopResults(InstrumentResults::FileItems& files, MessagePipe& pipe, IConnectorActionCallback* callback) {
	if (callback) callback->InstrumentDataReceiveFilesBegin();

    size_t filesSize;
	if(!pipe.read(&filesSize))
		return false;

	if (callback) callback->InstrumentDataReceiveFilesCount(filesSize);

	size_t step = max(1, filesSize / 100);

	bool result = true;
    files.resize(filesSize);
    for(size_t i = 0; result && i < filesSize; ++i) 
	{
        InstrumentResults::FileItem& item = files[i];
		result = 
			pipe.read(&item.fileId) &&
			pipe.read(&item.fileUrl);

		if (i % step == 0 && callback) callback->InstrumentDataReceiveFilesStat(i);
    }

	if (callback) callback->InstrumentDataReceiveFilesEnd();

	return result;
}

bool PushResults(InstrumentResults::AssemblyResults& assemblies, MessagePipe& pipe)
{
	if(!pipe.write(assemblies.size()))
		return false;

    bool result = true;
    for(InstrumentResults::AssemblyResults::const_iterator asmIt = assemblies.begin(); 
		asmIt != assemblies.end() && result; 
		asmIt++) 
	{
        const InstrumentResults::AssemblyResult& asmResult = *asmIt;

		result = 
			pipe.write(asmResult.name) &&
			pipe.write(asmResult.moduleName) &&
			pipe.write(asmResult.types.size());

        for(InstrumentResults::TypedefResults::const_iterator typeIt = asmResult.types.begin(); 
			typeIt != asmResult.types.end() && result; 
			typeIt++) 
		{
            const InstrumentResults::TypedefResult& typedefResult = *typeIt;

			result = 
				pipe.write(typedefResult.fullName) &&
				pipe.write(typedefResult.flags) &&
				pipe.write(typedefResult.methods.size());

            for(InstrumentResults::MethodResults::const_iterator methodIt = typedefResult.methods.begin();
				methodIt != typedefResult.methods.end() && result; 
				methodIt++)
			{
                const InstrumentResults::MethodResult& methodResult = *methodIt;

				result = 
					pipe.write(methodResult.name) &&
					pipe.write(methodResult.sig) &&
					pipe.write(methodResult.flags) &&
					pipe.write(methodResult.implFlags) &&
					pipe.write(methodResult.blocks.size());

                for(InstrumentResults::MethodBlocks::const_iterator blockIt = methodResult.blocks.begin(); 
					blockIt != methodResult.blocks.end() && result; 
					blockIt++) 
				{
                    const InstrumentResults::MethodBlock& block = *blockIt;

					result = 
						pipe.write(block.position) &&
						pipe.write(block.blockLength) &&
						pipe.write(block.visitCount) &&
						pipe.write(block.haveSource);

                    if (result && block.haveSource) {
						result = 
							pipe.write(block.sourceFileId) &&
							pipe.write(block.startLine) &&
							pipe.write(block.startColumn) &&
							pipe.write(block.endLine) &&
							pipe.write(block.endColumn);
                    }
                }
            }
        }
    }
	return result;
}

bool PopResults(InstrumentResults::AssemblyResults& assemblies, MessagePipe& pipe, IConnectorActionCallback* callback) 
{
	if (callback) callback->InstrumentDataReceiveCountersBegin();

	size_t assemblySize;
	if(!pipe.read(&assemblySize))
		return false;

	if (callback) callback->InstrumentDataReceiveCountersAsmCount(assemblySize);
    assemblies.resize(assemblySize);

	bool result = true;
    for(size_t i = 0; i < assemblySize; ++i) 
	{
		size_t typesCount = 0;

        InstrumentResults::AssemblyResult& asmResult = assemblies[i];

		result = 
			pipe.read(&asmResult.name) &&
			pipe.read(&asmResult.moduleName) &&
			pipe.read(&typesCount);

		if (result) {
			asmResult.types.resize(typesCount);
			if (callback) callback->InstrumentDataReceiveCountersAsm(CComBSTR(asmResult.name.c_str()), CComBSTR(asmResult.moduleName.c_str()), asmResult.types.size());
		}

        for(size_t j = 0; result && j < typesCount; ++j) 
		{
            size_t methodCount;

            InstrumentResults::TypedefResult& typedefResult = asmResult.types[j];

			result = 
				pipe.read(&typedefResult.fullName) &&
				pipe.read(&typedefResult.flags) &&
				pipe.read(&methodCount);

			if (result)
	            typedefResult.methods.resize(methodCount);

            for(size_t m = 0; result && m < methodCount; ++m)
			{
                size_t blockCount;

                InstrumentResults::MethodResult& methodResult = typedefResult.methods[m];

				result = 
					pipe.read(&methodResult.name) &&
					pipe.read(&methodResult.sig) &&
					pipe.read(&methodResult.flags) &&
					pipe.read(&methodResult.implFlags) &&
					pipe.read(&blockCount);

                if (result)
					methodResult.blocks.resize(blockCount);

                for(size_t b = 0; result && b < blockCount; ++b) 
				{
                    InstrumentResults::MethodBlock& block = methodResult.blocks[b];

					result = 
						pipe.read(&block.position) &&
						pipe.read(&block.blockLength) &&
						pipe.read(&block.visitCount) &&
						pipe.read(&block.haveSource);

                    if (result && block.haveSource) {
						result = 
							pipe.read(&block.sourceFileId) &&
							pipe.read(&block.startLine) &&
							pipe.read(&block.startColumn) &&
							pipe.read(&block.endLine) &&
							pipe.read(&block.endColumn);
                    }
                }
            }
        }
    }
	if (callback) callback->InstrumentDataReceiveCountersEnd();
	return result;
}

bool InstrumentResults::SendData(MessagePipe& pipe) {
    ATLTRACE("InstrumentResults::SendResults - send eResult");
    return PushResults(m_results, pipe) 
		&& PushResults(m_fileTable, pipe);
}

bool InstrumentResults::ReceiveData(MessagePipe& pipe) {
	return ReceiveData(pipe, 0);
}

bool InstrumentResults::ReceiveData(MessagePipe& pipe, IConnectorActionCallback* callback) {
    ATLTRACE("InstrumentResults::SendResults - receive result");
	if(callback) callback->InstrumentDataReceiveBegin();

    m_results.clear();
    m_fileTable.clear();

	if (!PopResults(m_results, pipe, callback))
		return false;

	if(!PopResults(m_fileTable, pipe, callback))
		return false;

	if(callback) callback->InstrumentDataReceiveEnd();

	return true;
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
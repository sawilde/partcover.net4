#include "StdAfx.h"
#include "interface.h"
#include "logging.h"
#include "instrumented_results.h"

void InstrumentResults::GetReport(IReportReceiver& walker) {
    for(SkippedItems::const_iterator itemIt = m_skippedItems.begin(); itemIt != m_skippedItems.end(); itemIt++) {
        const SkippedItem& item = *itemIt;
		CComBSTR assemblyName(item.assemblyName.c_str());
		CComBSTR typedefName(item.typedefName.c_str());
		if(FAILED(walker.RegisterSkippedItem(assemblyName, typedefName)))
            return;
    }

	BLOCK_DATA data;

    for(AssemblyResults::const_iterator asmIt = m_results.begin(); asmIt != m_results.end(); asmIt++) {
        const AssemblyResult& asmResult = *asmIt;

        CComBSTR asmName(asmResult.name.c_str());
        CComBSTR moduleName(asmResult.module.c_str());
		CComBSTR domainName(asmResult.domain.c_str());
		if(FAILED(walker.EnterAssembly(asmResult.domainIndex, domainName, asmName, moduleName)))
            return;

        for(TypedefResults::const_iterator typeIt = asmResult.types.begin(); typeIt != asmResult.types.end(); typeIt++) {
            const TypedefResult& typedefResult = *typeIt;

            CComBSTR typedefName(typedefResult.fullName.c_str());
			if(FAILED(walker.EnterTypedef(typedefName, typedefResult.flags)))
                return;

            for(MethodResults::const_iterator methodIt = typedefResult.methods.begin(); methodIt != typedefResult.methods.end(); methodIt++) {
                const MethodResult& methodResult = *methodIt;

                CComBSTR methodName(methodResult.name.c_str());
                CComBSTR methodSig(methodResult.sig.c_str());
				if(FAILED(walker.EnterMethod(methodName, methodSig, methodResult.bodySize, 
                    methodResult.flags, methodResult.implFlags, methodResult.symbolFileId, methodResult.methodDef)))
                    return;

                for(MethodBlocks::const_iterator blockIt = methodResult.blocks.begin(); blockIt != methodResult.blocks.end(); blockIt++) {
                    const MethodBlock& block = *blockIt;

					data.position = block.position;
					data.blockLen = block.blockLength;
					data.visitCount = block.visitCount;

					if(FAILED(walker.AddCoverageBlock(data)))
					{
                        return;
					}
                }

				if(FAILED(walker.LeaveMethod()))
				{
                    return;
				}
			}
		
			if(FAILED(walker.LeaveTypedef())) 
			{
                return;
			}
		}

		if(FAILED(walker.LeaveAssembly()))
		{
            return;
		}
   }
}

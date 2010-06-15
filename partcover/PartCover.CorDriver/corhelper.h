#pragma once

#include "helpers.h"
#include "il_sigparser.h"

namespace CorHelper {

	String GetAppDomainName(ICorProfilerInfo* info, AppDomainID domain);
    String GetAssemblyName(ICorProfilerInfo* info, AssemblyID assembly);
    String GetModuleName(ICorProfilerInfo* info, ModuleID module);

    String GetTypedefFullName(IMetaDataImport* mdImport, mdTypeDef typeDef, DWORD *p_typeDefFlags, const String& connectStr = _T(""), const String& innerTypeDefName = _T(""));
	String TypeRefName(IMetaDataImport* mdImport, mdTypeRef tr);

    String GetClassName(ICorProfilerInfo* info, ClassID classId);
	String GetMethodPath(ICorProfilerInfo2* info, FunctionID func, COR_PRF_FRAME_INFO frame) ;

	String GetMethodName(IMetaDataImport* mdImport, mdMethodDef methodDef, DWORD* attrs, DWORD* implFlags) ;

	bool LoadMethodSig(ICorProfilerInfo* info, IMetaDataImport* mdImport, mdMethodDef methodDef, DynamicArray<sig_byte>& sig);
    void ParseMethodSig(ICorProfilerInfo* info, IMetaDataImport* mdImport, mdMethodDef methodDef, String* sigVal);
}

#pragma once

namespace CorHelper {
    String GetModuleName(ICorProfilerInfo* info, ModuleID module);
    String GetAssemblyName(ICorProfilerInfo* info, AssemblyID assembly);

    String GetTypedefFullName(IMetaDataImport* mdImport, mdTypeDef typeDef, DWORD *p_typeDefFlags, const String& connectStr = _T(""), const String& innerTypeDefName = _T(""));
	String TypeRefName(IMetaDataImport* mdImport, mdTypeRef tr);

    String GetClassName(ICorProfilerInfo* info, ClassID classId);

	String GetMethodName(IMetaDataImport* mdImport, mdMethodDef methodDef, DWORD* attrs, DWORD* implFlags) ;
    void GetMethodSig(ICorProfilerInfo* info, IMetaDataImport* mdImport, mdMethodDef methodDef, String* sigVal);
}

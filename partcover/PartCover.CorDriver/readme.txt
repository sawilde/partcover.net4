HOW TO

// get assembly name 
CorHelper::GetAssemblyName(ICorProfilerInfo*, AssemblyID);

// get typedef name
CorHelper::GetTypedefFullName(helper.mdImport, typeDef, &typeDefFlags);

// extract namespace 
RulesHelpers::ExtractNamespace(typedefName);

// get methoddef name
CorHelper::GetMethodName(helper.mdImport, methodDef, NULL, NULL);

// get signature of method
CorHelper::GetMethodSig(helper.profilerInfo, helper.mdImport, methodDef, &methodSig);

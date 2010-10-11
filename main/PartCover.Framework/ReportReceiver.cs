using System;
using System.Reflection;
using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public class ReportReceiver : IReportReceiver
    {
        public Report Report { get; set; }

        public void RegisterFile(int fileId, string fileUrl)
        {
            Report.Files.Add(new FileEntry
            {
                Id = fileId,
                PathUri = fileUrl
            });
        }

        public void RegisterSkippedItem(string assemblyName, string typedefName)
        {
            Report.SkippedItems.Add(new SkippedEntry 
            {
                AssemblyName = assemblyName,
                TypedefName = typedefName
            });
        }

        private AssemblyEntry currentAssembly;
        public void EnterAssembly(int domainIndex, string domainName, string assemblyName, string moduleName)
        {
            Report.Assemblies.Add(currentAssembly = new AssemblyEntry
            {
                AssemblyRef = Report.Assemblies.Count + 1,
                Module = moduleName,
                Name = assemblyName,
                Domain = domainName,
                DomainIndex = domainIndex
            });
        }

        private TypedefEntry currentTypedef;
        public void EnterTypedef(string typedefName, uint flags)
        {
            currentAssembly.Types.Add(currentTypedef = new TypedefEntry
            {
                Assembly = currentAssembly,
                Name = typedefName,
                Attributes = (TypeAttributes)flags
            });
        }

        private MethodEntry currentMethod;
        public void EnterMethod(string methodName, string methodSig, int bodySize, int bodyLineCount, int bodySeqCount, uint flags, uint implFlags)
        {
            currentTypedef.Methods.Add(currentMethod = new MethodEntry
            {
                Type = currentTypedef,
                Name = methodName,
                Signature = methodSig,
                BodySize = bodySize,
                BodyLineCount = bodyLineCount,
                BodySeqCount = bodySeqCount,
                Flags = (MethodAttributes)flags,
                ImplFlags = (MethodImplAttributes)implFlags
            });
        }

        public void AddCoverageBlock(BLOCK_DATA blockData)
        {
            currentMethod.Blocks.Add(new MethodBlock
            {
                File = Math.Max(0, blockData.fileId),
                Offset = blockData.position,
                Length = blockData.blockLen,
                VisitCount = blockData.visitCount,
                Start = new Position { Column = blockData.startColumn, Line = blockData.startLine },
                End = new Position { Column = blockData.endColumn, Line = blockData.endLine },
            });
        }

        public void LeaveMethod() { }

        public void LeaveTypedef() { }

        public void LeaveAssembly() { }
    }
}
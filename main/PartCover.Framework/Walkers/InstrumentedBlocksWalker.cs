using System;

namespace PartCover.Framework.Walkers
{
    /*
    internal class InstrumentedBlocksWalkerInner :
        InstrumentedBlocksWalker,
        IReportReceiver
    {
        readonly CoverageReport coverage;
        public CoverageReport Report
        {
            get { return coverage; }
        }

        public InstrumentedBlocksWalkerInner()
        {
            coverage = new CoverageReport();
        }

        public void BeginReport() { }

        public void EndReport() { }

        public void RegisterFile(uint fileId, String fileUrl)
        {
            CoverageReportHelper.AddFile(Report, fileId, fileUrl);
        }

        CoverageReport.TypeDescriptor currentType;

        public void EnterTypedef(String assemblyName, String typedefName, UInt32 flags)
        {
            currentType = new CoverageReport.TypeDescriptor
            {
                assemblyName = assemblyName,
                typeName = typedefName,
                flags = flags
            };
        }

        public void LeaveTypedef()
        {
            CoverageReportHelper.AddType(Report, currentType);
        }

        CoverageReport.MethodDescriptor currentMethod;

        public void EnterMethod(String methodName, String methodSig, UInt32 flags, UInt32 implFlags)
        {
            currentMethod = new CoverageReport.MethodDescriptor(1)
            {
                methodName = methodName,
                methodSig = methodSig,
                flags = flags,
                implFlags = implFlags
            };
        }

        public void LeaveMethod()
        {
            CoverageReportHelper.AddMethod(currentType, currentMethod);
        }

        public void MethodBlock(UInt32 position, UInt32 blockLen, UInt32 visitCount, UInt32 fileId, UInt32 startLine, UInt32 startColumn, UInt32 endLine, UInt32 endColumn)
        {
            var inner = new CoverageReport.InnerBlock
            {
                position = position,
                blockLen = blockLen,
                visitCount = visitCount,
                fileId = fileId,
                startLine = startLine,
                startColumn = startColumn,
                endLine = endLine,
                endColumn = endColumn
            };
            CoverageReportHelper.AddMethodBlock(currentMethod, inner);
        }
    }
     */
}

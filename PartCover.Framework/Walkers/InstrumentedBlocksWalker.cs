using System;
using System.Diagnostics;
using System.Collections;
using System.Xml;

using PartCover.Framework.Walkers;

namespace PartCover.Framework.Walkers
{
    public interface InstrumentedBlocksWalker
    {
        CoverageReport Report { get; }
    }

    internal class InstrumentedBlocksWalkerInner :
        InstrumentedBlocksWalker,
        PartCover.IInstrumentedBlockWalker
    {
        CoverageReport coverage;
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

        public void RegisterFile(System.UInt32 fileId, System.String fileUrl)
        {
            CoverageReportHelper.AddFile(Report, fileId, fileUrl);
        }

        CoverageReport.TypeDescriptor currentType;

        public void EnterTypedef(String assemblyName, String typedefName, UInt32 flags)
        {
            currentType = new CoverageReport.TypeDescriptor();
            currentType.assemblyName = assemblyName;
            currentType.typeName = typedefName;
            currentType.flags = flags;
        }

        public void LeaveTypedef()
        {
            CoverageReportHelper.AddType(Report, currentType);
        }

        CoverageReport.MethodDescriptor currentMethod;

        public void EnterMethod(String methodName, String methodSig,  UInt32 flags, UInt32 implFlags)
        {
            currentMethod = new CoverageReport.MethodDescriptor(1);
            currentMethod.methodName = methodName;
            currentMethod.methodSig = methodSig;
            currentMethod.flags = flags;
            currentMethod.implFlags = implFlags;
        }

        public void LeaveMethod()
        {
            CoverageReportHelper.AddMethod(currentType, currentMethod);
        }

        public void MethodBlock(UInt32 position, UInt32 blockLen, UInt32 visitCount, UInt32 fileId, UInt32 startLine, UInt32 startColumn, UInt32 endLine, UInt32 endColumn)
        {
            CoverageReport.InnerBlock inner = new CoverageReport.InnerBlock();
            inner.position = position;
            inner.blockLen = blockLen;
            inner.visitCount = visitCount;
            inner.fileId = fileId;
            inner.startLine = startLine;
            inner.startColumn = startColumn;
            inner.endLine = endLine;
            inner.endColumn = endColumn;

            CoverageReportHelper.AddMethodBlock(currentMethod, inner);
        }
    }
}

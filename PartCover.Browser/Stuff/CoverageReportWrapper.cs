using System;
using System.Collections.Generic;
using System.Text;

using PartCover.Browser.Api;
using PartCover.Browser.Api.ReportItems;
using PartCover.Framework.Walkers;
using System.Reflection;
using PartCover.Browser.Stuff.ReportItems;

namespace PartCover.Browser.Stuff
{
    class CoverageReportWrapper : ICoverageReport
    {
        readonly CoverageReport report;
        public CoverageReport Report
        {
            get { return report; }
        }

        public CoverageReportWrapper(CoverageReport report)
        {
            this.report = report;
        }

        List<AssemblyItem> assemblyList = new List<AssemblyItem>();
        public IAssembly[] getAssemblies()
        {
            return assemblyList.ToArray();
        }

        public void build()
        {
            foreach (string asmName in CoverageReportHelper.GetAssemblies(report))
            {
                AssemblyItem assemblyItem = new AssemblyItem(asmName);
                foreach (CoverageReport.TypeDescriptor d in CoverageReportHelper.GetTypes(report, assemblyItem.Name))
                {
                    ClassItem classItem = new ClassItem(d.typeName, assemblyItem);
                    buildNamespaceChain(assemblyItem, classItem);
                    buildMethods(d.methods, classItem);
                    assemblyItem.addType(classItem);
                }
                assemblyList.Add(assemblyItem);
            }
        }

        private void buildMethods(CoverageReport.MethodDescriptor[] mdList, ClassItem classItem)
        {
            foreach (CoverageReport.MethodDescriptor md in mdList)
            {
                MethodItem mdItem = new MethodItem(md, classItem);
                buildMethodBlocks(md, mdItem);
                classItem.addMethod(mdItem);
            }
        }

        private void buildMethodBlocks(CoverageReport.MethodDescriptor md, MethodItem mdItem)
        {
            foreach (CoverageReport.InnerBlockData ibd in md.insBlocks)
            {
                CoveredVariantItem cvItem = new CoveredVariantItem();
                cvItem.Blocks = ibd.blocks;
                mdItem.addBlock(cvItem);
            }
        }

        private void buildNamespaceChain(AssemblyItem assemblyItem, ClassItem classItem)
        {
            string[] parts = CoverageReportHelper.SplitNamespaces(classItem.QName);

            NamespaceItem lastNamespaceItem = null;
            for (int i = 0; i < parts.Length - 1; ++i)
            {
                NamespaceItem namespaceItem = assemblyItem.findNamespace(parts[i], lastNamespaceItem);
                if (namespaceItem == null)
                {
                    namespaceItem = new NamespaceItem(parts[i], assemblyItem);
                    namespaceItem.Parent = lastNamespaceItem;
                    assemblyItem.addNamespace(namespaceItem);
                }
                lastNamespaceItem = namespaceItem;
            }

            classItem.Namespace = lastNamespaceItem;
        }

        public string getFilePath(uint file)
        {
            return CoverageReportHelper.GetFileUrl(report, file);
        }

        public void forEachBlock(Action<CoverageReport.InnerBlock> blockReceiver)
        {
            report.forEachInnerBlock(blockReceiver);
        }

        public ICollection<CoverageReport.RunHistoryMessage> getRunHistory()
        {
            return report.runHistory;
        }

        public ICollection<CoverageReport.RunLogMessage> getLogEvents()
        {
            return report.runLog;
        }

        public int? getExitCode()
        {
            return report.ExitCode;
        }
    }
}

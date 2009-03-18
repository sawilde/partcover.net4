using System;
using System.Collections.Generic;

using PartCover.Browser.Api;
using PartCover.Browser.Api.ReportItems;
using PartCover.Framework.Walkers;
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

        readonly List<AssemblyItem> assemblyList = new List<AssemblyItem>();
        public ICollection<IAssembly> Assemblies
        {
            get
            {
                return assemblyList.ToArray();
            }
        }

        public void build()
        {
            foreach (var asmName in CoverageReportHelper.GetAssemblies(report))
            {
                var assemblyItem = new AssemblyItem(asmName);
                foreach (var d in CoverageReportHelper.GetTypes(report, assemblyItem.Name))
                {
                    var classItem = new ClassItem(d.typeName, assemblyItem);
                    BuildNamespaceChain(assemblyItem, classItem);
                    BuildMethods(d.methods, classItem);
                    assemblyItem.addType(classItem);
                }
                assemblyList.Add(assemblyItem);
            }
        }

        private static void BuildMethods(IEnumerable<CoverageReport.MethodDescriptor> mdList, ClassItem classItem)
        {
            foreach (var md in mdList)
            {
                var mdItem = new MethodItem(md, classItem);
                BuildMethodBlocks(md, mdItem);
                classItem.addMethod(mdItem);
            }
        }

        private static void BuildMethodBlocks(CoverageReport.MethodDescriptor md, MethodItem mdItem)
        {
            foreach (var ibd in md.insBlocks)
            {
                var cvItem = new CoveredVariantItem
                {
                    Blocks = ibd.blocks
                };
                mdItem.addBlock(cvItem);
            }
        }

        private static void BuildNamespaceChain(AssemblyItem assemblyItem, ClassItem classItem)
        {
            var parts = CoverageReportHelper.SplitNamespaces(classItem.QName);

            NamespaceItem lastNamespaceItem = null;
            for (var i = 0; i < parts.Length - 1; ++i)
            {
                var namespaceItem = assemblyItem.findNamespace(parts[i], lastNamespaceItem);
                if (namespaceItem == null)
                {
                    namespaceItem = new NamespaceItem(parts[i], assemblyItem)
                    {
                        Parent = lastNamespaceItem
                    };
                    assemblyItem.addNamespace(namespaceItem);
                }
                lastNamespaceItem = namespaceItem;
            }

            classItem.Namespace = lastNamespaceItem;
        }

        public string ResolveFilePath(uint file)
        {
            return CoverageReportHelper.GetFileUrl(report, file);
        }

        public void ForEachBlock(Action<CoverageReport.InnerBlock> blockReceiver)
        {
            report.forEachInnerBlock(blockReceiver);
        }

        public ICollection<CoverageReport.RunHistoryMessage> RunHistory
        {
            get { return report.runHistory; }
        }

        public ICollection<CoverageReport.RunLogMessage> LogEvents
        {
            get { return report.runLog; }
        }

        public int? ExitCode
        {
            get { return report.ExitCode; }
        }
    }
}

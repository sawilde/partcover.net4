using System.Collections.Generic;
using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public class ReportDomainFlattener
    {
        public Report Report { get; private set; }

        public ReportDomainFlattener(Report report)
        {
            Report = report;
        }

        public Report Flatten()
        {
            var report = Report.Copy();

            var assemblyNames = new List<string>();
            report.Assemblies.ForEach(x => { if (!assemblyNames.Contains(x.Name)) assemblyNames.Add(x.Name); });

            var flattenEntries = new List<AssemblyEntry>();
            foreach (var name in assemblyNames)
            {
                var assemblyName = name;
                flattenEntries.Add(Flatten(report.Assemblies.FindAll(x => x.Name == assemblyName)));
            }

            report.Assemblies.Clear();
            report.Assemblies.AddRange(flattenEntries);
            return report;
        }

        public static AssemblyEntry Flatten(List<AssemblyEntry> entries)
        {
            var entry = new AssemblyEntry
            {
                AssemblyRef = entries[0].AssemblyRef,
                Domain = entries[0].Domain,
                DomainIndex = 1,
                Module = entries[0].Module,
                Name = entries[0].Name
            };


            //store all types
            var typedefEntries = new List<TypedefEntry>();
            entries.ForEach(x => x.Types.ForEach(typedefEntries.Add));

            //select unique type names
            var typedefNames = new List<string>();
            typedefEntries.ForEach(x => { if (!typedefNames.Contains(x.Name)) typedefNames.Add(x.Name); });

            //flatten type information
            foreach (var name in typedefNames)
            {
                var typedefName = name;
                entry.Types.Add(Flatten(entry, typedefEntries.FindAll(x => x.Name == typedefName)));
            }

            return entry;
        }

        private static TypedefEntry Flatten(AssemblyEntry assembly, List<TypedefEntry> list)
        {
            var entry = list[0].Copy(assembly);
            if (list.Count == 1) return entry;

            foreach (var typedefEntry in list.GetRange(1, list.Count - 1))
            {
                CopyCoverage(entry, typedefEntry);
            }
            return entry;
        }

        private static void CopyCoverage(TypedefEntry dst, TypedefEntry src)
        {
            foreach (var el in src.Methods)
            {
                var srcEntry = el;
                var dstEntry = dst.Methods.Find(x => x.Name == srcEntry.Name && x.Signature == srcEntry.Signature);
                CopyCoverage(dstEntry, srcEntry);
            }
        }

        private static void CopyCoverage(MethodEntry dst, MethodEntry src)
        {
            foreach (var el in src.Blocks)
            {
                var srcBlock = el;
                var dstBlock = dst.Blocks.Find(x => x.Offset == srcBlock.Offset);
                if (dstBlock == null)
                {
                    dst.Blocks.Add(srcBlock.Copy());
                }
                else
                {
                    dstBlock.VisitCount = srcBlock.VisitCount;
                }
            }
        }
    }
}
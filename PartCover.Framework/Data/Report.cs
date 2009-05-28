using System;
using System.Collections.Generic;

namespace PartCover.Framework.Data
{
    public class Report
    {
        public Report()
        {
            Files = new List<FileEntry>();
            Assemblies = new List<AssemblyEntry>();
            SkippedItems = new List<SkippedEntry>();
            Date = DateTime.Now;
        }

        public List<FileEntry> Files { get; private set; }
        public List<AssemblyEntry> Assemblies { get; private set; }
        public List<SkippedEntry> SkippedItems { get; private set; }
        public DateTime Date { get; set; }

        public string ResolveFilePath(int file)
        {
            return Files.Find(x => x.Id == file).PathUri;
        }

        public Report Copy()
        {
            var copy = new Report { Date = Date };

            copy.Files.AddRange(Files.ConvertAll(x => x.Copy()));
            copy.Assemblies.AddRange(Assemblies.ConvertAll(x => x.Copy()));

            return copy;
        }
    }
}
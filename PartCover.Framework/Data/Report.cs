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
            Date = DateTime.Now;
        }

        public List<FileEntry> Files { get; private set; }
        public List<AssemblyEntry> Assemblies { get; private set; }
        public DateTime Date { get; set; }

        public string ResolveFilePath(int file)
        {
            return Files.Find(x => x.Id == file).PathUri;
        }
    }
}
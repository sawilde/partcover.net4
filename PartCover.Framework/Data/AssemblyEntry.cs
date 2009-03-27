using System.Collections.Generic;

namespace PartCover.Framework.Data
{
    public class AssemblyEntry
    {
        public AssemblyEntry()
        {
            Types = new List<TypedefEntry>();
        }

        public string Name { get; set; }

        public string Module { get; set; }

        public List<TypedefEntry> Types { get; private set; }
    }
}
using System.Collections.Generic;

namespace PartCover.Framework.Data
{
    public class AssemblyEntry
    {
        public AssemblyEntry()
        {
            Types = new List<TypedefEntry>();
        }

        public int AssemblyRef { get; set; }

        public string Name { get; set; }
        public string Module { get; set; }
        public string Domain { get; set; }
        public int DomainIndex { get; set; }
        public List<TypedefEntry> Types { get; private set; }

        public AssemblyEntry Copy()
        {
            var copy = new AssemblyEntry
            {
                AssemblyRef = AssemblyRef,
                Name = Name,
                Module = Module,
                Domain = Domain,
                DomainIndex = DomainIndex
            };
            copy.Types = new List<TypedefEntry>(Types.ConvertAll(x => x.Copy(copy)));

            return copy;
        }
    }
}
using System.Collections.Generic;
using System.Reflection;

namespace PartCover.Framework.Data
{
    public class TypedefEntry
    {
        public TypedefEntry()
        {
            Methods = new List<MethodEntry>();
        }

        public AssemblyEntry Assembly { get; set; }

        public string Name { get; set; }

        public TypeAttributes Attributes { get; set; }

        public List<MethodEntry> Methods { get; set; }

        public TypedefEntry Copy(AssemblyEntry assembly)
        {
            var copy = new TypedefEntry
            {
                Assembly = assembly,
                Name = Name,
                Attributes = Attributes
            };

            copy.Methods = new List<MethodEntry>(Methods.ConvertAll(x => x.Copy(copy)));
            return copy;
        }
    }
}
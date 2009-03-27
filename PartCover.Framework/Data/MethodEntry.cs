using System.Collections.Generic;
using System.Reflection;

namespace PartCover.Framework.Data
{
    public class MethodEntry
    {
        public MethodEntry() { 
            Blocks = new List<MethodBlock>();
        }

        public string Name { get; set; }

        public MethodAttributes Flags { get; set; }

        public MethodImplAttributes ImplFlags { get; set; }

        public string Signature { get; set; }

        public List<MethodBlock> Blocks { get; private set; }

        public TypedefEntry Type { get; set; }
    }
}
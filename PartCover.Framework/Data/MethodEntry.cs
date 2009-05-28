using System.Collections.Generic;
using System.Reflection;

namespace PartCover.Framework.Data
{
    public class MethodEntry
    {
        public MethodEntry()
        {
            Blocks = new List<MethodBlock>();
        }

        public string Name { get; set; }

        public MethodAttributes Flags { get; set; }

        public MethodImplAttributes ImplFlags { get; set; }

        public string Signature { get; set; }

        public List<MethodBlock> Blocks { get; private set; }

        public TypedefEntry Type { get; set; }

        public MethodEntry Copy(TypedefEntry type)
        {
            return new MethodEntry
            {
                Type = type,
                Name = Name,
                Flags = Flags,
                ImplFlags = ImplFlags,
                Signature = Signature,
                Blocks = new List<MethodBlock>(Blocks.ConvertAll(x => x.Copy()))
            };
        }
    }
}
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
        public string Signature { get; set; }
        public int BodySize { get; set; }
        public MethodAttributes Flags { get; set; }
        public MethodImplAttributes ImplFlags { get; set; }

        public List<MethodBlock> Blocks { get; private set; }
        public TypedefEntry Type { get; set; }

        public MethodEntry Copy(TypedefEntry type)
        {
            return new MethodEntry
            {
                Type = type,
                Name = Name,
                Flags = Flags,
                BodySize = BodySize,
                ImplFlags = ImplFlags,
                Signature = Signature,
                Blocks = new List<MethodBlock>(Blocks.ConvertAll(x => x.Copy()))
            };
        }
    }
}
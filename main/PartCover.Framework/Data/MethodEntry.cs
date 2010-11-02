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
        public int SymbolFileId { get; set; }
        public MethodAttributes Flags { get; set; }
        public MethodImplAttributes ImplFlags { get; set; }

        public int MethodDef { get; set; }

        public List<MethodBlock> Blocks { get; private set; }
        public TypedefEntry Type { get; set; }

        public MethodEntry Copy(TypedefEntry type)
        {
            return new MethodEntry
            {
                Type = type,
                Name = this.Name,
                Flags = this.Flags,
                BodySize = this.BodySize,
                ImplFlags = this.ImplFlags,
                Signature = this.Signature,
                SymbolFileId = this.SymbolFileId,
                MethodDef = this.MethodDef,
                Blocks = new List<MethodBlock>(Blocks.ConvertAll(x => x.Copy()))
            };
        }
    }
}
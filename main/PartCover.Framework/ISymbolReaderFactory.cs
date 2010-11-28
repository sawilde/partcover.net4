using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics.SymbolStore;

namespace PartCover.Framework
{
    public interface ISymbolReaderFactory
    {
        ISymbolReader GetSymbolReader(string moduleName);
    }
}

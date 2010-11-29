using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics.SymbolStore;

namespace PartCover.Framework
{
    public class SymbolReaderFactory : ISymbolReaderFactory
    {
        private SymBinder _symBinder;

        public SymbolReaderFactory()
        {
            _symBinder = new SymBinder();
        }

        public ISymbolReader GetSymbolReader(string moduleName)
        {
            return SymbolReaderWapper.GetSymbolReader(_symBinder, moduleName);
        }
    }
}

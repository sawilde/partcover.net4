using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using System.Diagnostics.SymbolStore;
using System.IO;

namespace PartCover.Framework.UnitTests
{
    [TestFixture]
    public class SymbolReaderWrapperTests
    {
        [Test]
        public void GetSymbolReader_ReturnsNull_WhenFileNotFound()
        {
            var binder = new SymBinder();
            var x = SymbolReaderWapper.GetSymbolReader(binder, "");
            Assert.IsNull(x);
        }

        [Test]
        public void GetSymbolReader_ReturnsNotNull_WhenFileFound()
        {
            var location = Path.Combine(Environment.CurrentDirectory, "PartCover.StressTest.exe");

            var binder = new SymBinder();
            var x = SymbolReaderWapper.GetSymbolReader(binder, location);
            Assert.IsNotNull(x);
        }

        
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using System.Diagnostics.Contracts;

namespace PartCover.TestTarget
{
    internal class ContractClass
    {
        private bool _x;
        private bool _y;

        public ContractClass(bool x)
        {
            _x = x;
            _y = true;
        }

        public bool RunContract(string data)
        {
            Contract.Requires(data != null);
            Contract.Ensures(_x != false);

            return _x;
        }
    }

    /// <summary>
    /// partcover.exe 
    /// --target ..\..\libraries\nunit\NUnit-2.5.5.10112\bin\net-2.0\nunit-console.exe 
    /// --target-args "PartCover.TestTarget.dll /include=Contract" 
    /// --include [Part*]* 
    /// --output coverage.xml
    /// </summary>
    [TestFixture, Category("Contract")]
    public class ContractTests
    {
        [Test]
        public void Contract_Throws_ContractException()
        {
            try
            {
                new ContractClass(true).RunContract("data");
                Assert.Fail();
            }
            catch (Exception)
            {

            } 
        }
    }
}

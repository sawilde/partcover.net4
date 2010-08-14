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
        public static bool RunContract(string data)
        {
            Contract.Requires(data != null);
            Contract.Ensures(data != null);
            Contract.Ensures(data != string.Empty);
            return data.Contains("fred");
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
        public void Contract_Bombs_Under_Profiling()
        {
            ContractClass.RunContract("data");
        }
    }
}

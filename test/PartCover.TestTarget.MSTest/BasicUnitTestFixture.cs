using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using PartCover.Target;

namespace PartCover.TestTarget.MSTest
{
    

    [TestClass]
    public class BasicUnitTestFixture
    {
        [TestMethod]
        public void FirstTest()
        {
            MyTargetClass.MyMethod();
            Assert.Fail("Arrgghhh!");
        }
    }
}

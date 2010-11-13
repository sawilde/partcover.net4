using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using System.IO;
using System.Reflection;

namespace PartCover.Framework.UnitTests
{
    [TestFixture]
    public class ReportReceiverTests
    {
        ReportReceiver receiver;

        [SetUp]
        public void SetUp()
        {
            receiver = new ReportReceiver { Report = new Data.Report() };
        }


        [Test]
        public void RegisterSkippedItem_Adds_EntriesToReport()
        {
            //arrange
             
            //act
            receiver.RegisterSkippedItem("AssemblyName", "TypeDefName");

            //assert
            Assert.AreEqual(1, receiver.Report.SkippedItems.Count);
        }

        [Test]
        public void EnterAssembly_Adds_AssemblyToReport()
        {
            //arrange

            //act
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");

            //assert
            Assert.AreEqual(1, receiver.Report.Assemblies.Count);
            Assert.AreEqual("AssemblyName", receiver.Report.Assemblies[0].Name);
        }

        [Test]
        public void EnterTypedef_Adds_TypeToCurrentAssemblyToReport()
        {
            //arrange
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");

            //act
            receiver.EnterTypedef("TypedefName", 0);

            //assert
            Assert.AreEqual(1, receiver.Report.Assemblies[0].Types.Count);
            Assert.AreEqual("TypedefName", receiver.Report.Assemblies[0].Types[0].Name);
        }

        [Test]
        public void EnterMethod_Adds_MethodToCurrentTypeToReport()
        {
            //arrange
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");
            receiver.EnterTypedef("TypedefName", 0);

            //act
            receiver.EnterMethod("MethodName", "MethodSig", 0, 0, 0, 0, 0);

            //assert
            Assert.AreEqual(1, receiver.Report.Assemblies[0].Types[0].Methods.Count);
            Assert.AreEqual("MethodName", receiver.Report.Assemblies[0].Types[0].Methods[0].Name);
        }

        [Test]
        public void AddCoverageBlock_Adds_BlockToCurrentMethodToReport()
        {
            //arrange
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");
            receiver.EnterTypedef("TypedefName", 0);
            receiver.EnterMethod("MethodName", "MethodSig", 0, 0, 0, 0, 0);
            var blockData = new BLOCK_DATA { position = 99, visitCount = 0, blockLen = 0 };

            //act
            receiver.AddCoverageBlock(blockData);

            //assert
            Assert.AreEqual(1, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks.Count);
            Assert.AreEqual(99, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[0].Offset);
        }

        [Test]
        public void LeaveMethod_Performs_LengthCalculationBetweenLastBlockAndBodySize()
        {
            //arrange
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");
            receiver.EnterTypedef("TypedefName", 0);
            receiver.EnterMethod("MethodName", "MethodSig", 10, 0, 0, 0, 0);
            receiver.AddCoverageBlock(new BLOCK_DATA { position = 0, visitCount = 0 });

            //act
            receiver.LeaveMethod();

            //receive
            Assert.AreEqual(10, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[0].Length);
            
        }

        [Test]
        public void LeaveMethod_Performs_LengthCalculationBetweenBlocks()
        {
            //arrange
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");
            receiver.EnterTypedef("TypedefName", 0);
            receiver.EnterMethod("MethodName", "MethodSig", 10, 0, 0, 0, 0);
            receiver.AddCoverageBlock(new BLOCK_DATA { position = 0, visitCount = 0 });
            receiver.AddCoverageBlock(new BLOCK_DATA { position = 3, visitCount = 0 });
            receiver.AddCoverageBlock(new BLOCK_DATA { position = 7, visitCount = 0 });

            //act
            receiver.LeaveMethod();

            //assert
            Assert.AreEqual(3, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[0].Length);
            Assert.AreEqual(4, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[1].Length);
            Assert.AreEqual(3, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[2].Length);
        }

        [Test]
        public void EnterAssembly_Adds_FilesToReport_If_SymbolDataFound()
        {
            //arrange
            
            //act
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "PartCover.StressTest.exe");
            
            //assert
            Assert.AreEqual(5, receiver.Report.Files.Count);
        }

        [Test]
        public void EnterMethod_BuildsBlocks_If_SymbolDataFound()
        {
            //arrange
            var location = Path.Combine(Environment.CurrentDirectory, "PartCover.StressTest.exe");
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", location);
            receiver.EnterTypedef("TypedefName", 0);

            var ass = Assembly.LoadFrom(location);
            MethodInfo m = ass.GetType("PartCover.StressTest.Program")
                .GetMethod("Main", BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Instance);
            
            //act
            receiver.EnterMethod("MethodName", "MethodSig", 10, 119, 0, 0, m.MetadataToken);

            //assert
            Assert.AreEqual(7, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks.Count);
        }

        [Test]
        public void AddCoverageVlock_UpdatesExistingBlocks_If_BlockDataFound()
        {
            //arrange
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "ModuleName");
            receiver.EnterTypedef("TypedefName", 0);
            receiver.EnterMethod("MethodName", "MethodSig", 10, 119, 0, 0, 10);
            receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks.Add(new Data.MethodBlock() { Offset = 0, VisitCount = 10 });
            receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks.Add(new Data.MethodBlock() { Offset = 3, VisitCount = 5 });

            //act
            receiver.AddCoverageBlock(new BLOCK_DATA() { position = 0, visitCount = 9 });
            receiver.AddCoverageBlock(new BLOCK_DATA() { position = 3, visitCount = 7 });

            //assert
            Assert.AreEqual(9, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[0].VisitCount);
            Assert.AreEqual(7, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks[1].VisitCount);
        }

    }
}

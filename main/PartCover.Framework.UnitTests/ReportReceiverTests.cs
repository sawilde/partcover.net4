using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using System.IO;
using System.Reflection;
using Moq;
using System.Diagnostics.SymbolStore;

namespace PartCover.Framework.UnitTests
{
    [TestFixture]
    public class ReportReceiverTests
    {
        ReportReceiver receiver;
        Mock<ISymbolReaderFactory> _factory;

        [SetUp]
        public void SetUp()
        {
            _factory = new Mock<ISymbolReaderFactory>();
            receiver = new ReportReceiver(_factory.Object) { Report = new Data.Report() };
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
        public void EnterAssembly_Adds_FilesToReport()
        {
            //arrange
            var reader = new Mock<ISymbolReader>();
            var document = new Mock<ISymbolDocument>();

            document.SetupGet(x => x.URL).Returns("FileName");

            reader.Setup(x => x.GetDocuments()).Returns(new[] { document.Object });

            _factory.Setup(x => x.GetSymbolReader("Test.exe")).Returns(reader.Object);

            //act
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "Test.exe");
            
            //assert
            Assert.AreEqual(1, receiver.Report.Files.Count);
        }

        [Test]
        public void EnterAssembly_Adds_FilesToReport_IgnoresEmptyFiles()
        {
            //arrange
            var reader = new Mock<ISymbolReader>();
            var document = new Mock<ISymbolDocument>();

            document.SetupGet(x => x.URL).Returns("");

            reader.Setup(x => x.GetDocuments()).Returns(new[] { document.Object });

            _factory.Setup(x => x.GetSymbolReader("Test.exe")).Returns(reader.Object);

            //act
            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "Test.exe");

            //assert
            Assert.AreEqual(0, receiver.Report.Files.Count);
        }

        [Test]
        public void EnterMethod_BuildsBlocks_If_SymbolDataFound()
        {
            //arrange
            var reader = new Mock<ISymbolReader>();
            var method = new Mock<ISymbolMethod>();
            var document = new Mock<ISymbolDocument>();

            document.SetupGet(x => x.URL).Returns("FileName");

            reader.Setup(x => x.GetMethod(new SymbolToken(1234))).Returns(method.Object);

            method.SetupGet(x => x.SequencePointCount).Returns(1);

            method
                .Setup(x => x.GetSequencePoints(It.IsAny<int[]>(),
                    It.IsAny<ISymbolDocument[]>(),
                    It.IsAny<int[]>(),
                    It.IsAny<int[]>(),
                    It.IsAny<int[]>(),
                    It.IsAny<int[]>()))
                .Callback<int[], ISymbolDocument[], int[], int[], int[], int[]>((o, d, s1, s2, s3, s4) => 
                    {
                        o[0] = 1;
                        d[0] = document.Object;
                        s1[0] = 2;
                        s2[0] = 3;
                        s3[0] = 4;
                        s4[0] = 5;
                    }
                );

            _factory.Setup(x => x.GetSymbolReader(It.IsAny<string>())).Returns(reader.Object);

            receiver.EnterAssembly(0, "DomainName", "AssemblyName", "Test.exe");
            receiver.EnterTypedef("TypedefName", 0);

            //act
            receiver.EnterMethod("MethodName", "MethodSig", 10, 119, 0, 0, 1234);

            //assert
            Assert.AreEqual(1, receiver.Report.Assemblies[0].Types[0].Methods[0].Blocks.Count);
        }

        [Test]
        public void AddCoverageBlock_UpdatesExistingBlocks_If_BlockDataFound()
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

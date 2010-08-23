using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using Moq;

namespace PartCover.TestTarget
{
    public interface IDemoOne
    {
        int DoSomething(bool b);
    }

    public class DemoClass
    {
        private IDemoOne _One;
        public DemoClass(IDemoOne one)
        {
            _One = one;
        }

        public int ExerciseInterface()
        {
            return _One.DoSomething(true);
        }
    }

    [TestFixture]
    public class MoqTests
    {
        [Test]
        public void Does_ExerciseInterface_Call_DoSomething()
        {
            //arrange
            var mock = new Mock<IDemoOne>();

            mock.Setup(x => x.DoSomething(It.IsAny<bool>()))
                .Returns<bool>(b => b ? 1 : 0);

            var demo = new DemoClass(mock.Object);

            //act
            Assert.AreEqual(1, demo.ExerciseInterface());

            //assert
            mock.Verify(x => x.DoSomething(true), Times.Once());
            mock.Verify(x => x.DoSomething(false), Times.Never());
        }
    }
}

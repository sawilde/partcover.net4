using System;
using PartCover.StressTest.Classes;
using Moq;
namespace PartCover.StressTest
{
    class Program
    {
        interface IDo
        {
            int Something(string data);
        }

        static void Main(string[] args)
        {
            //new Line001().Foo();
            new BigMethod().Foo(new Random(DateTime.Now.Millisecond).Next(255));
            new GenericClass<string>("x").DoAction<float>(1);
            new GenericClass<int>(1).DoAction<string>("somedata");
            new Subclass().Method("data");
            new SubclassGeneric<int>().Method(1);
            
            var mock = new Mock<IDo>();
            mock.Setup(x => x.Something(It.IsAny<string>()))
                .Returns<string>(s => 1);

        }
    }
}

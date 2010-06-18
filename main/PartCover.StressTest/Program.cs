using System;
using PartCover.StressTest.Classes;

namespace PartCover.StressTest
{
    class Program
    {
        static void Main(string[] args)
        {
            new Line001().Foo();
            new BigMethod().Foo(new Random(DateTime.Now.Millisecond).Next(255));
            new GenericClass<string>("x").DoAction<float>(1);
            new GenericClass<int>(1).DoAction<string>("somedata");
            new Subclass().Method("data");
            new SubclassGeneric<int>().Method(1);

        }
    }
}

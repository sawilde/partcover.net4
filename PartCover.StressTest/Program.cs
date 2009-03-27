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
        }
    }
}

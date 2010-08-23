using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;

namespace PartCover.TestTarget
{
    public class GenericClass<T>
    {
        public GenericClass(T data)
        {
            Data = data;
        }

        public T Data { get; set; }
    }

    [TestFixture]
    public class GenericTests
    {
        [Test]
        public void InstantiateTest()
        {
            var data = (new GenericClass<string>("data")).Data;
            new GenericClass<int>(1);
            new GenericClass<float>(1.2f);
        }
    }
}

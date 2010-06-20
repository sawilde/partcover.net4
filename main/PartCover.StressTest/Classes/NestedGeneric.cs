using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PartCover.StressTest.Classes
{
    public class NestedGeneric
    {
        public class InnerTest<T>
        {
            private int _Val;

            public InnerTest(int val)
            {
                _Val = val;
            }

        }
    }
}

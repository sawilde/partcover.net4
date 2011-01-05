using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PartCover.Target
{
    public class MyTargetClass
    {
        public static void MyMethod()
        {
            for (int i = 0; i < 10; i++)
            {
                System.Diagnostics.Trace.WriteLine(string.Format("i={0}", i));
            }
        }
    }
}

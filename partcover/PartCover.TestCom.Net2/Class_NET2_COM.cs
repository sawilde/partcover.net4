using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace PartCover.TestCom.Net2
{
    [Guid("F8B68F25-D46F-4829-A2DA-C3A018CAA842")]
    public interface IDispInterface2
    {
        [DispId(1)]
        void WhatEnvironment();
    }

    [Guid("C58D4F65-5643-464D-9BD6-3044D3576296")]
    public class Class_NET2_COM : IDispInterface2
    {
        public Class_NET2_COM()
        {
        }

        public void WhatEnvironment()
        {
            Console.WriteLine("Hello from " + System.Environment.Version.ToString());
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics; 

namespace PartCover.TestCom.Net4
{
    [Guid("7DD641ED-5429-4DFB-82A0-4D4F4153A72F")]
    public interface IDispInterface4
    {
        [DispId(1)]
        void WhatEnvironment();
    }

    [Guid("2ABC95D3-0980-4138-B43D-17CF1DB84B51")]
    public class Class_NET4_COM : IDispInterface4
    {
        public Class_NET4_COM()
        {
        }

        public void WhatEnvironment()
        {
            Console.WriteLine("Hello from " + System.Environment.Version.ToString());
        }
    }
}

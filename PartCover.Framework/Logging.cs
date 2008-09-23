using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Framework
{
    [Flags]
    public enum Logging
    {
        Nothing = 0,
        DumpMethod = 2,//long one
        DumpInstrumentation = 4,//long one
        MethodInstrumentation = 8,
        MethodInner = 16,
        SkipByState = 32,
        SkipByRules = 64,
    }
}

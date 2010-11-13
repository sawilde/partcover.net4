using System;
using System.Collections.Generic;
//using System.Linq;
using System.Text;

namespace PartCover.StressTest.Classes
{
    public class NoReporting
    {
        private bool _notUsed;

        public NoReporting()
        {
            _notUsed = true;
        }

        public bool UnusedMethod()
        {
            return true;
        }
    }
}

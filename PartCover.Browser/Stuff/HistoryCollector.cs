using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Framework;
using PartCover.Browser.Api;

namespace PartCover.Browser.Stuff
{
    internal class HistoryCollector : IProgressCallback
    {
        private IProgressCallback proxy;
        private IRunHistory runHistory;

        public HistoryCollector(IProgressCallback proxy, IRunHistory runHistory)
        {
            this.proxy = proxy;
            this.runHistory = runHistory;
        }

        public void writeStatus(string value)
        {
            if (runHistory != null) runHistory.addMessage(value);
            if (proxy != null) proxy.writeStatus(value);
        }
    }
}

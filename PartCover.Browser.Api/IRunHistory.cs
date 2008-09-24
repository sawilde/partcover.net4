using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Framework.Walkers;
using PartCover.Framework;

namespace PartCover.Browser.Api
{
    public interface IRunHistory
    {
        event EventHandler<EventArgs<CoverageReport.RunHistoryMessage>> RunItemAdded;
        event EventHandler<EventArgs<CoverageReport.RunLogMessage>> EventItemAdded;
        event EventHandler<EventArgs> Cleanup;
        event EventHandler<EventArgs> ExitCodeChanged;

        void clear();

        CoverageReport.RunHistoryMessage[] RunItems { get;}
        CoverageReport.RunLogMessage[] EventItems { get;}

        int? ExitCode { get;set;}
    }
}

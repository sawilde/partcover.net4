using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Api
{
    public interface ICoverageReportService
    {
        event EventHandler<EventArgs> ReportClosing;
        event EventHandler<EventArgs> ReportOpened;

        ICoverageReport Report { get;}
        IRunHistory RunHistory { get;}

        void loadFromFile(string fileName, IProgressTracker tracker);

        void saveReport(string fileName, IProgressTracker tracker);

        void load(CoverageReport report, IProgressTracker tracker);
    }
}

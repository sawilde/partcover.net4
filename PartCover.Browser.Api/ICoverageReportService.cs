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

        void loadFromFile(string fileName);

        void saveReport(string fileName);

        void load(CoverageReport report);
    }
}

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
        string ReportFileName { get;}

        void LoadFromFile(string fileName);

        void SaveToFile(string fileName);

        void Load(CoverageReport report);
    }
}

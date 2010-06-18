using System;
using PartCover.Framework.Data;

namespace PartCover.Browser.Api
{
    public interface IReportService
    {
        event EventHandler ReportClosing;
        event EventHandler ReportOpened;

        Report Report { get; }
        string LastRunLog { get; }
        string ReportFileName { get; }

        void LoadFromFile(string fileName);
        void SaveToFile(string fileName);

        void Open(Report report);
    }
}

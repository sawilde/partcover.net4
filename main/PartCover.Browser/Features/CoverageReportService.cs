using System;
using System.Text;
using System.Xml;
using PartCover.Browser.Api;
using PartCover.Framework;
using PartCover.Framework.Data;

namespace PartCover.Browser.Features
{
    public class CoverageReportService
        : IFeature
        , IReportService
    {
        public event EventHandler ReportClosing;
        public event EventHandler ReportOpened;

        public Report Report { get; internal set; }
        public string LastRunLog { get; internal set; }
        public string ReportFileName { get; private set; }

        public void LoadFromFile(string fileName)
        {
            var report = new Report();
            using (var reader = new XmlTextReader(fileName))
            {
                ReportSerializer.Load(reader, report);
            }
            SetReport(report);
            ReportFileName = fileName;
        }

        private void SetReport(Report report)
        {
            if (Report != null && ReportClosing != null)
                ReportClosing(this, EventArgs.Empty);
            ReportFileName = null;
            Report = report;
            if (Report != null && ReportOpened != null)
                ReportOpened(this, EventArgs.Empty);
        }

        public void SaveToFile(string fileName)
        {
            using (var writer = new XmlTextWriter(fileName, Encoding.UTF8))
            {
                ReportSerializer.Save(writer, Report);
                ReportFileName = fileName;
            }
        }

        public void Open(Report report)
        {
            SetReport(report);
        }

        public void Attach(IServiceContainer container) { }

        public void Detach(IServiceContainer container) { }

        public void Build(IServiceContainer container) { }

        public void Destroy(IServiceContainer container) { }

    }
}

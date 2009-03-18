using System;
using PartCover.Browser.Api;
using PartCover.Framework.Walkers;
using System.IO;
using PartCover.Browser.Stuff;

namespace PartCover.Browser.Features
{
    public class CoverageReportService
        : IFeature
        , ICoverageReportService
    {
        public event EventHandler<EventArgs> ReportClosing;
        public event EventHandler<EventArgs> ReportOpened;

        CoverageReportWrapper reportWrapper;
        public ICoverageReport Report
        {
            get { return reportWrapper; }
        }

        public string ReportFileName { get; private set; }

        public void LoadFromFile(string fileName)
        {
            var report = new CoverageReport();
            using (var reader = new StreamReader(fileName))
            {
                CoverageReportHelper.ReadReport(report, reader);
            }
            setReport(report);
            ReportFileName = fileName;
        }

        private void setReport(CoverageReport report)
        {
            if (Report != null && ReportClosing != null)
                ReportClosing(this, EventArgs.Empty);

            var wrapper = new CoverageReportWrapper(report);
            wrapper.build();

            ReportFileName = null;
            reportWrapper = wrapper;
            if (Report != null && ReportOpened != null)
                ReportOpened(this, EventArgs.Empty);
        }

        public void SaveToFile(string fileName)
        {
            using (var writer = new StreamWriter(fileName))
            {
                CoverageReportHelper.WriteReport(reportWrapper.Report, writer);
                ReportFileName = fileName;
            }
        }

        public void Load(CoverageReport report)
        {
            setReport(report);
        }

        public void Attach(IServiceContainer container) { }

        public void Detach(IServiceContainer container) { }

        public void Build(IServiceContainer container) { }

        public void Destroy(IServiceContainer container) { }

    }
}

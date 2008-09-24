using System;
using System.Collections.Generic;
using System.Text;
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

        public void loadFromFile(string fileName)
        {
            CoverageReport report = new CoverageReport();
            using (StreamReader reader = new StreamReader(fileName))
            {
                CoverageReportHelper.ReadReport(report, reader);
            }
            setReport(report);
        }

        private void setReport(CoverageReport report)
        {
            if (Report != null && ReportClosing != null)
                ReportClosing(this, EventArgs.Empty);

            CoverageReportWrapper wrapper = new CoverageReportWrapper(report);
            wrapper.build();

            reportWrapper = wrapper;
            if (Report != null && ReportOpened != null)
                ReportOpened(this, EventArgs.Empty);
        }

        public void saveReport(string fileName)
        {
            using (StreamWriter writer = new StreamWriter(fileName))
            {
                CoverageReportHelper.WriteReport(reportWrapper.Report, writer);
            }
        }

        public void load(CoverageReport report)
        {
            setReport(report);
        }

        public void attach(IServiceContainer container) { }

        public void detach(IServiceContainer container) { }

        public void build(IServiceContainer container) { }

        public void destroy(IServiceContainer container) { }

    }
}

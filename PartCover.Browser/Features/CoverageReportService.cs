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

        readonly CoverageReportRunHistory history = new CoverageReportRunHistory();
        public IRunHistory RunHistory
        {
            get { return history; }
        }

        public void loadFromFile(string fileName, IProgressTracker tracker)
        {
            history.clear();
            CoverageReport report = new CoverageReport();
            using (StreamReader reader = new StreamReader(fileName))
            {
                CoverageReportHelper.ReadReport(report, reader);
            }
            setReport(report, tracker);
        }

        private void setReport(CoverageReport report, IProgressTracker tracker)
        {
            if (Report != null && ReportClosing != null)
                ReportClosing(this, EventArgs.Empty);

            CoverageReportWrapper wrapper = new CoverageReportWrapper(report);
            wrapper.build(tracker);

            reportWrapper = wrapper;
            if (Report != null && ReportOpened != null)
                ReportOpened(this, EventArgs.Empty);
        }

        public void saveReport(string fileName, IProgressTracker tracker)
        {
            using (StreamWriter writer = new StreamWriter(fileName))
            {
                CoverageReportHelper.WriteReport(reportWrapper.Report, writer);
            }
        }

        public void load(CoverageReport report, IProgressTracker tracker)
        {
            setReport(report, tracker);
        }

        public void attach(IServiceContainer container) { }

        public void detach(IServiceContainer container) { }

        public void build(IServiceContainer container) { }

        public void destroy(IServiceContainer container) { }

    }
}

using System;

using PartCover.Browser.Api;

namespace PartCover.Browser.Features
{
    public class FeatureReportItemSelection
        : IFeature
        , IReportItemSelectionService
    {
        public void Attach(IServiceContainer container) { }

        public void Detach(IServiceContainer container) { }

        public void Build(IServiceContainer container)
        {
            container.getService<ICoverageReportService>().ReportClosing += onReportClosing;
            container.getService<ICoverageReportService>().ReportOpened += onReportOpened;
        }

        public void Destroy(IServiceContainer container)
        {
            container.getService<ICoverageReportService>().ReportClosing -= onReportClosing;
            container.getService<ICoverageReportService>().ReportOpened -= onReportOpened;
        }

        public IReportItem SelectedItem { get; private set; }

        static void onReportOpened(object sender, EventArgs e) { }

        static void onReportClosing(object sender, EventArgs e) { }

        public void Select<T>(T item) where T : IReportItem
        {
            SelectedItem = item;
            fireSelectionChanged();
        }

        public void SelectNone()
        {
            SelectedItem = null;
            fireSelectionChanged();
        }

        private void fireSelectionChanged()
        {
            if (SelectionChanged != null) SelectionChanged(this, EventArgs.Empty);
        }

        public event EventHandler SelectionChanged;
    }
}

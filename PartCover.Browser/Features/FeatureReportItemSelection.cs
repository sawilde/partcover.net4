using System;
using System.Collections.Generic;
using System.Text;

using PartCover.Browser.Api;
using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Features
{
    public class FeatureReportItemSelection
        : IFeature
        , IReportItemSelectionService
    {
        public void attach(IServiceContainer container) { }

        public void detach(IServiceContainer container) { }

        public void build(IServiceContainer container)
        {
            container.getService<ICoverageReportService>().ReportClosing += onReportClosing;
            container.getService<ICoverageReportService>().ReportOpened += onReportOpened;
        }

        public void destroy(IServiceContainer container)
        {
            container.getService<ICoverageReportService>().ReportClosing -= onReportClosing;
            container.getService<ICoverageReportService>().ReportOpened -= onReportOpened;
        }

        private IReportItem selectedItem;
        public IReportItem SelectedItem
        {
            get { return selectedItem; }
        }

        void onReportOpened(object sender, EventArgs e) { }

        void onReportClosing(object sender, EventArgs e) { }

        public void select<T>(T item) where T : IReportItem
        {
            selectedItem = item;
            fireSelectionChanged();
        }

        public void selectNone()
        {
            selectedItem = null;
            fireSelectionChanged();
        }

        private void fireSelectionChanged()
        {
            if (SelectionChanged != null) SelectionChanged(this, EventArgs.Empty);
        }

        public event EventHandler SelectionChanged;
    }
}

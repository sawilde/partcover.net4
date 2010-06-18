using PartCover.Browser.Api;
using PartCover.Browser.Features.Views;

namespace PartCover.Browser.Features
{
    public class FeatureViewCoverage
        : IFeature
        , IReportViewFactory
    {
        public void Build(IServiceContainer container)
        {
            container.getService<IReportViewValve>().add(this);
        }

        public void Destroy(IServiceContainer container)
        {
            container.getService<IReportViewValve>().remove(this);
        }

        public void Attach(IServiceContainer container) { }

        public void Detach(IServiceContainer container) { }

        public ReportView Create()
        {
            return new CoverageView();
        }

        public string ViewName
        {
            get { return "View Coverage Details"; }
        }
    }
}

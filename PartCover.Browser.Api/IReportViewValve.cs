namespace PartCover.Browser.Api
{
    public interface IReportViewValve
    {
        void add(IReportViewFactory factory);
        void remove(IReportViewFactory factory);
    }
}

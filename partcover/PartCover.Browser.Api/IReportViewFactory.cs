namespace PartCover.Browser.Api
{
    public interface IReportViewFactory
    {
        ReportView Create();

        string ViewName { get; }
    }
}

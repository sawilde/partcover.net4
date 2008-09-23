using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public interface IReportViewFactory
    {
        ReportView create();

        string ViewName { get; }
    }
}

using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public interface IReportViewValve
    {
        void add(IReportViewFactory factory);
        void remove(IReportViewFactory factory);
    }
}

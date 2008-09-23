using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Api.ReportItems
{
    public interface ICoveredVariant : IReportItem
    {
        CoverageReport.InnerBlock[] Blocks { get; }
    }
}

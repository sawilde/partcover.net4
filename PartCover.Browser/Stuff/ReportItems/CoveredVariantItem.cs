using System;
using System.Collections.Generic;
using System.Text;

using PartCover.Browser.Api.ReportItems;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Stuff.ReportItems
{
    class CoveredVariantItem : ICoveredVariant
    {
        CoverageReport.InnerBlock[] blocks;

        public CoverageReport.InnerBlock[] Blocks
        {
            get { return blocks ?? new CoverageReport.InnerBlock[0]; }
            set { blocks = value; }
        }
    }
}

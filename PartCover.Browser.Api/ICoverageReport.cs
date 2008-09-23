using System;
using System.Collections.Generic;
using System.Text;

using PartCover.Browser.Api.ReportItems;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Api
{
    public interface ICoverageReport
    {
        IAssembly[] getAssemblies();

        string getFilePath(uint file);

        void forEachBlock(Action<CoverageReport.InnerBlock> blockReceiver);
    }
}

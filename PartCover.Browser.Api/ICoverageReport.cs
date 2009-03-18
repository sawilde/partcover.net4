using System;
using System.Collections.Generic;

using PartCover.Browser.Api.ReportItems;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Api
{
    public interface ICoverageReport
    {
        ICollection<IAssembly> Assemblies { get; }

        string ResolveFilePath(uint file);

        void ForEachBlock(Action<CoverageReport.InnerBlock> blockReceiver);

        ICollection<CoverageReport.RunHistoryMessage> RunHistory { get; }

        ICollection<CoverageReport.RunLogMessage> LogEvents { get; }

        int? ExitCode { get; }
    }
}

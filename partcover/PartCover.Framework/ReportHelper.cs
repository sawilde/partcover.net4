using System;
using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public class ReportHelper
    {
        public static float GetBlockCoverage(MethodBlock block)
        {
            return block.Length == 0 || block.VisitCount > 0 ? 1 : 0;
        }

        public static void ForEachBlock(Report report, Action<MethodBlock> blockVisitor)
        {
            report.Assemblies.ForEach(a =>
                a.Types.ForEach(t =>
                    t.Methods.ForEach(m =>
                        m.Blocks.ForEach(blockVisitor))));
        }
    }
}
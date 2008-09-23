using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api.ReportItems
{
    public interface IClass : IReportItem
    {
        uint Flags { get;}

        string Name { get;}

        IAssembly Assembly { get;}

        IMethod[] getMethods();

        INamespace[] getNamespaceChain();
    }
}

using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api.ReportItems
{
    public interface IAssembly : IReportItem
    {
        IClass[] getTypes();

        string Name { get;}
    }
}

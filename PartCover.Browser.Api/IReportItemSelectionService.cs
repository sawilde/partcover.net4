using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Api
{
    public interface IReportItemSelectionService
    {
        void select<T>(T asm) where T : IReportItem;

        void selectNone();

        IReportItem SelectedItem { get;}

        event EventHandler SelectionChanged;
    }
}

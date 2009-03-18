using System;

namespace PartCover.Browser.Api
{
    public interface IReportItemSelectionService
    {
        void Select<T>(T asm) where T : IReportItem;

        void SelectNone();

        IReportItem SelectedItem { get;}

        event EventHandler SelectionChanged;
    }
}

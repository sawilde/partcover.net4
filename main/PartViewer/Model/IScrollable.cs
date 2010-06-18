using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace PartViewer.Model
{
    internal interface IScrollable
    {
        void setScroll(Size bounds, int h, int v);
    }
}

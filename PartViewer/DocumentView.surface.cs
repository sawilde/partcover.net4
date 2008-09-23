using System.Drawing;

namespace PartViewer
{
    internal class DocumentViewSurface
    {
        private readonly TextControl target;

        public DocumentViewSurface(TextControl target)
        {
            this.target = target;
        }

        public Graphics Graphics { get { return target.Graphics; } }

        public void invalidate()
        {
            target.invalidate();
        }

        public void setScroll(Size bounds, int h, int v)
        {
            target.setScroll(bounds, h, v);
        }

        public bool hasFocus()
        {
            return target.Focused;
        }
    }
}

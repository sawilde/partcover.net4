using System.Drawing;

namespace PartViewer.Styles
{
    public class Style
    {
        private string fontName;
        private float fontHeight;

        private Color foreground;
        private Color background;

        private FontStyle fontStyle;

        public string FontName
        {
            get { return fontName; }
            set { fontName = value; }
        }

        public float FontHeight
        {
            get { return fontHeight; }
            set { fontHeight = value; }
        }

        public FontStyle FontStyle
        {
            get { return fontStyle; }
            set { fontStyle = value; }
        }

        public Color Foreground
        {
            get { return foreground; }
            set { foreground = value; }
        }

        public Color Background
        {
            get { return background; }
            set { background = value; }
        }

        public virtual Style combine(Style style)
        {
            Style result = new Style();

            result.fontHeight = fontHeight;
            result.fontName = fontName;
            result.foreground = foreground;
            result.background = background;

            if (style.fontName != null)
                result.fontName = style.fontName;
            if (style.fontHeight > 0)
                result.fontHeight = style.fontHeight;
            if (!style.foreground.IsEmpty)
                result.foreground = style.foreground;
            if (!style.background.IsEmpty)
                result.background = style.background;

            result.fontStyle |= style.fontStyle;

            return result;
        }
    }
}

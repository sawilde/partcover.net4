using System.Drawing;

namespace PartViewer.Model
{
    public class Style
    {
        public string FontName { get; set; }
        public float FontHeight { get; set; }
        public FontStyle FontStyle { get; set; }
        public Color Foreground { get; set; }
        public Color Background { get; set; }

        public virtual Style Combine(Style style)
        {
            var result = new Style
            {
                FontHeight = FontHeight,
                FontName = FontName,
                Foreground = Foreground,
                Background = Background
            };

            if (style.FontName != null)
                result.FontName = style.FontName;
            if (style.FontHeight > 0)
                result.FontHeight = style.FontHeight;
            if (!style.Foreground.IsEmpty)
                result.Foreground = style.Foreground;
            if (!style.Background.IsEmpty)
                result.Background = style.Background;

            result.FontStyle |= style.FontStyle;

            return result;
        }
    }
}

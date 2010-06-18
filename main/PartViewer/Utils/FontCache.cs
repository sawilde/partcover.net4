using System.Collections.Generic;
using System.Drawing;
using PartViewer.Model;

namespace PartViewer.Utils
{
    internal static class FontCache
    {
        static readonly Dictionary<string, Font> fontCache = new Dictionary<string, Font>();

        public static Font get(Style style)
        {
            Font font;
            if (!fontCache.TryGetValue(uidOf(style), out font))
            {
                fontCache[uidOf(style)] = font = createFont(style);
            }
            return font;
        }

        private static string uidOf(Style style)
        {
            return style.FontName + ", "
                + style.FontHeight + "pt, "
                + style.FontStyle;
        }

        private static Font createFont(Style style)
        {
            return new Font(style.FontName, style.FontHeight, style.FontStyle);
        }
    }
}

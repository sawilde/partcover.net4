using System.Collections.Generic;
using System.Drawing;
using PartViewer.Styles;

namespace PartViewer.Cache
{
    internal static class BrushCache
    {
        static readonly Dictionary<Color, SolidBrush> solidCache = new Dictionary<Color, SolidBrush>();

        public static Brush getSolid(Color color)
        {
            SolidBrush brush;
            if (!solidCache.TryGetValue(color, out brush))
            {
                solidCache[color] = brush = createSolid(color);
            }
            return brush;
        }

        public static Brush getForeground(Style style)
        {
            return getSolid(style.Foreground);
        }

        private static SolidBrush createSolid(Color color)
        {
            return new SolidBrush(color);
        }
    }
}

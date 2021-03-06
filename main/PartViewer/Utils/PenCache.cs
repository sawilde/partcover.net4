using System.Collections.Generic;
using System.Drawing;

namespace PartViewer.Utils
{
    public static class PenCache
    {
        static readonly Dictionary<Color, Pen> solidCache = new Dictionary<Color, Pen>();

        public static Pen GetSolid(Color color)
        {
            Pen pen;
            if (!solidCache.TryGetValue(color, out pen))
            {
                solidCache[color] = pen = createSolid(color);
            }
            return pen;
        }

        private static Pen createSolid(Color color)
        {
            return new Pen(color);
        }
    }
}

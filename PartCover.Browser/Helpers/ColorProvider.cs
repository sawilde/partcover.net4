using System;
using System.Drawing;
using System.Collections.Generic;

namespace PartCover.Browser.Helpers
{
    public enum BlockStyle { 
        Bad,
        Good
    }

	public sealed class ColorProvider
	{
		private ColorProvider() {}

        public static Color GetForeColorForPercent(float percent) {
            if (percent < 10) return Color.FromArgb(0xFF, 0x00, 0x00);
            if (percent < 20) return Color.FromArgb(0xF0, 0x00, 0x00);
            if (percent < 30) return Color.FromArgb(0xE0, 0x00, 0x00);
            if (percent < 40) return Color.FromArgb(0xD0, 0x00, 0x00);
            if (percent < 50) return Color.FromArgb(0xC0, 0x00, 0x00);
            if (percent < 60) return Color.FromArgb(0xB0, 0x00, 0x00);
            if (percent < 70) return Color.FromArgb(0xA0, 0x00, 0x00);
            if (percent < 80) return Color.FromArgb(0x90, 0x00, 0x00);
            if (percent < 90) return Color.FromArgb(0x80, 0x00, 0x00);
            return Color.FromArgb(0x00, 0x00, 0x00);
        }

        public static Color GetForeColorForBlock(float percent) {
            return (percent < 50) ? BlockColors[0] : BlockColors[1];
        }

        public readonly static Color[] BlockColors = new Color[] {
            Color.FromArgb(0xC0, 0x00, 0x00),
            Color.Empty
        };
    }
}

namespace PartViewer
{
    internal static class Native
    {
        /*         
                public enum BkMode
                {
                    TRANSPARENT = 1,
                    OPAQUE = 2
                }

                public static Color setBkColor(IDeviceContext ctx, Color color)
                {
                    IntPtr hdc = IntPtr.Zero;
                    try
                    {
                        hdc = ctx.GetHdc();
                        return Win32.SetBkColor(hdc, new Win32.COLORREF(color)).Color;
                    }
                    finally
                    {
                        if (hdc != IntPtr.Zero)
                        {
                            ctx.ReleaseHdc();
                        }
                    }
                }

                public static BkMode SetBkMode(IDeviceContext ctx, BkMode iBkMode)
                {
                    IntPtr hdc = IntPtr.Zero;
                    try
                    {
                        hdc = ctx.GetHdc();
                        return (BkMode)Win32.SetBkMode(hdc, (int)iBkMode);
                    }
                    finally
                    {
                        if (hdc != IntPtr.Zero)
                        {
                            ctx.ReleaseHdc();
                        }
                    }
                }

                public static class WindowStyle
                {
                    private const int GWL_STYLE = (-16);

                    public const long HScroll = 0x00100000;
                    public const long VScroll = 0x00200000;

                    public static bool haveStyle(long controlStyle, long style)
                    {
                        return (controlStyle & style) == style;
                    }

                    public static bool AddStyle(Control control, long style)
                    {
                        long oldStyle = GetStyle(control);
                        if (oldStyle == 0)
                            return false;

                        if ((oldStyle & style) == style)
                            return true;

                        oldStyle |= style;

                        return IntPtr.Zero == SetWindowLongPtr(control, GWL_STYLE, new IntPtr(oldStyle));
                    }

                    public static bool RemoveStyle(Control control, long style)
                    {
                        long oldStyle = GetStyle(control);
                        if (oldStyle == 0)
                            return false;

                        if ((oldStyle & style) == 0)
                            return true;

                        oldStyle &= ~style;

                        return IntPtr.Zero == SetWindowLongPtr(control, GWL_STYLE, new IntPtr(oldStyle));
                    }

                    public static long GetStyle(Control control)
                    {
                        return GetWindowLongPtr(control, GWL_STYLE).ToInt64();
                    }
                }

                public static IntPtr GetWindowLongPtr(Control control, int nIndex)
                {
                    switch (IntPtr.Size)
                    {
                        case 4:
                            return Win32.GetWindowLong(new HandleRef(control, control.Handle), nIndex);
                        case 8:
                            return Win32.GetWindowLongPtr(new HandleRef(control, control.Handle), nIndex);
                        default:
                            throw new NotImplementedException();
                    }
                }

                public static IntPtr SetWindowLongPtr(Control control, int nIndex, IntPtr dwNewLong)
                {
                    switch (IntPtr.Size)
                    {
                        case 4:
                            return Win32.SetWindowLong(new HandleRef(control, control.Handle), nIndex, dwNewLong);
                        case 8:
                            return Win32.SetWindowLongPtr(new HandleRef(control, control.Handle), nIndex, dwNewLong);
                        default:
                            throw new NotImplementedException();
                    }
                }

                private static class Win32
                {
                    [DllImport("user32.dll", EntryPoint = "SetWindowLongA", CharSet = CharSet.Ansi)]
                    public static extern IntPtr SetWindowLong(HandleRef hWnd, int nIndex, IntPtr dwNewLong);

                    [DllImport("user32.dll", EntryPoint = "SetWindowLongA", CharSet = CharSet.Ansi)]
                    public static extern IntPtr GetWindowLong(HandleRef hWnd, int nIndex);

                    [DllImport("user32.dll", EntryPoint = "SetWindowLongPtrA", CharSet = CharSet.Ansi)]
                    public static extern IntPtr SetWindowLongPtr(HandleRef hWnd, int nIndex, IntPtr dwNewLong);

                    [DllImport("user32.dll", EntryPoint = "GetWindowLongPtrA", CharSet = CharSet.Ansi)]
                    public static extern IntPtr GetWindowLongPtr(HandleRef hWnd, int nIndex);

                    [DllImport("gdi32.dll")]
                    public static extern COLORREF SetBkColor(IntPtr hdc, COLORREF crColor);

                    [DllImport("gdi32.dll")]
                    public static extern int SetBkMode(IntPtr hdc, int iBkMode);

                    // Alternate
                    [StructLayout(LayoutKind.Sequential)]
                    public struct COLORREF
                    {
                        public uint ColorDWORD;

                        public COLORREF(Color color)
                        {
                            ColorDWORD = ColorToRGB(color);
                        }

                        public Color Color
                        {
                            get { return RGBToColor(ColorDWORD); }
                            set { ColorDWORD = ColorToRGB(value); }
                        }
                    }

                    public static uint ColorToRGB(Color value)
                    {
                        return value.R + (((uint)value.G) << 8) + (((uint)value.B) << 16);
                    }

                    public static Color RGBToColor(uint rgb)
                    {
                        return Color.FromArgb((int)(0x000000FFU & rgb), (int)(0x0000FF00U & rgb) >> 8, (int)(0x00FF0000U & rgb) >> 16);
                    }
                }
                */
    }
}

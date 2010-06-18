using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace PartViewer.Utils
{
    public static class ClipboardManager
    {
        private const int NTries = 5;

        //private static class WinErrors
        //{
        //    public const long CLIPBRD_E_CANT_OPEN = 0x800401D0L;
        //    public const long CLIPBRD_E_CANT_EMPTY = 0x800401D1L;
        //    public const long CLIPBRD_E_CANT_CLOSE = 0x800401D4L;
        //    public const long CLIPBRD_E_CANT_SET = 0x800401D2L;
        //}
        //private static string GetErrorText(long errorCode)
        //{
        //    switch (errorCode)
        //    {
        //        case WinErrors.CLIPBRD_E_CANT_OPEN:
        //            return "Can't copy image into clipboard: can't open clipboard, another window has the clipboard open";
        //        case WinErrors.CLIPBRD_E_CANT_EMPTY:
        //            return "Can't copy image into clipboard: can't empty clipboard";
        //        case WinErrors.CLIPBRD_E_CANT_CLOSE:
        //            return "Can't copy image into clipboard: can't close clipboard";
        //        case WinErrors.CLIPBRD_E_CANT_SET:
        //            return "Can't copy image into clipboard: can't set clipboard";
        //        default:
        //            return "Can't copy image into clipboard: unknown error";
        //    }
        //}

        public static bool PutPlain(string text)
        {
            var tryCount = 0;
            while (tryCount++ < NTries)
            {
                try
                {
                    Clipboard.SetText(text, TextDataFormat.UnicodeText);
                    Clipboard.SetText(text, TextDataFormat.Text);
                    return true;
                }
                catch (ExternalException)
                {
                    //string error = GetErrorText(e.ErrorCode);
                }
            }

            return false;
        }
    }
}

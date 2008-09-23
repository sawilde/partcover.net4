using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace PartCover.Browser.Stuff
{
    internal static class HtmlPreview
    {
        const string XsltExt = ".xslt";
        const string XsltDirName = "xslt";

        public static string XsltDir
        {
            get { return Path.Combine(Environment.CurrentDirectory, XsltDirName); }
        }

        public static IEnumerable<string> enumTransforms()
        {
            string[] list;
            try
            {
                list = Directory.GetFiles(XsltDir, "*" + XsltExt);
            }
            catch (Exception e)
            {
                Trace.TraceError("Cannot get xslt folder\r\n{0}", e);
                yield break;
            }

            foreach (string s in list)
                yield return Path.GetFileNameWithoutExtension(s);
        }


    }
}

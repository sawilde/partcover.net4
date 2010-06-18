using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Xml.Xsl;
using PartCover.Browser.Api;

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


        public static void DoTransform(IProgressTracker tracker, string inputFile, string transform)
        {
            var xsltFilePath = Path.Combine(XsltDir, transform + XsltExt);

            tracker.AppendMessage("Load xslt file " + xsltFilePath);
            var tran = new XslCompiledTransform(false);
            tran.Load(xsltFilePath);

            tracker.AppendMessage("Transform report xml file");
            var outputFile = Path.GetTempFileName() + ".html";
            tran.Transform(inputFile, outputFile);

            tracker.AppendMessage("Open html report");
            Process.Start(outputFile);

            tracker.AppendMessage("Done");
        }
    }
}

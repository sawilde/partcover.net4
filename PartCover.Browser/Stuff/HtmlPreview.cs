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
            string xsltFilePath = Path.Combine(XsltDir, transform + XsltExt);

            tracker.setMessage("Load xslt file " + xsltFilePath);
            XslCompiledTransform tran = new XslCompiledTransform(false);
            tran.Load(xsltFilePath);

            tracker.setMessage("Transform report xml file");
            string outputFile = Path.GetTempFileName() + ".html";
            tran.Transform(inputFile, outputFile);

            tracker.setMessage("Open html report");
            Process.Start(outputFile);

            tracker.setMessage("Done");
        }
    }
}

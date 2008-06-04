using System;
using System.IO;
using System.Diagnostics;
using System.Xml;

using PartCover.Framework;
using PartCover.Framework.Walkers;

namespace PartCover
{
	class ApplicationEntry
	{
        [STAThread]
		static void Main(string[] args)
		{

            try {
                WorkSettings settings = new WorkSettings();
                if(!settings.InitializeFromArgs(args)) {
                    return;
                }

                Framework.Connector connector = new Framework.Connector();
                connector.SetVerbose(settings.LogLevel);

                foreach(string item in settings.IncludeItems) {
                    try {
                        connector.IncludeItem(item);
                    } catch(ArgumentException) {
                        System.Console.Error.WriteLine("Item '" + item + "' have wrong format");
                    }
                }

                foreach(string item in settings.ExcludeItems) {
                    try {
                        connector.ExcludeItem(item);
                    } catch(ArgumentException) {
                        System.Console.Error.WriteLine("Item '" + item + "' have wrong format");
                    }
                }

                connector.StartTarget(
                    settings.TargetPath, 
                    settings.TargetWorkingDir, 
                    settings.TargetArgs,
                    true,
                    false);

                try {
                    if ( settings.OutputToFile ) {
                        StreamWriter writer = File.CreateText(settings.FileNameForReport);
                        CoverageReportHelper.WriteReport(connector.BlockWalker.Report, writer);
                        writer.Close();
                    } else {
                        CoverageReportHelper.WriteReport(connector.BlockWalker.Report, System.Console.Out);
                    }
                } catch (Exception ex) {
                    System.Console.Error.WriteLine("Can't save report (" +ex.Message + ")");
                }

            } catch(SettingsException ex) {
                System.Console.Error.WriteLine(ex.Message);
            } catch(Exception ex) {
				System.Console.Error.WriteLine(ex.Message);
				System.Console.Error.WriteLine(ex.StackTrace);
			}
		}
	}
}

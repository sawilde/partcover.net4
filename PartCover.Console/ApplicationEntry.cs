using System;
using System.IO;

using PartCover.Framework;
using PartCover.Framework.Walkers;

namespace PartCover
{
    class ApplicationEntry
    {
        [STAThread]
        static int Main(string[] args)
        {
            try
            {
                WorkSettings settings = new WorkSettings();
                if (!settings.InitializeFromCommandLine(args))
                {
                    return -1;
                }

                Connector connector = new Connector();
                connector.OnEventMessage += connector_OnEventMessage;
                connector.ProcessCallback.OnMessage += ProcessCallback_OnMessage;

                connector.UseFileLogging(true);
                connector.UsePipeLogging(false);
                connector.SetLogging((Logging)settings.LogLevel);

                foreach (string item in settings.IncludeItems)
                {
                    try
                    {
                        connector.IncludeItem(item);
                    }
                    catch (ArgumentException)
                    {
                        Console.Error.WriteLine("Item '" + item + "' have wrong format");
                    }
                }

                foreach (string item in settings.ExcludeItems)
                {
                    try
                    {
                        connector.ExcludeItem(item);
                    }
                    catch (ArgumentException)
                    {
                        Console.Error.WriteLine("Item '" + item + "' have wrong format");
                    }
                }

                connector.StartTarget(
                    settings.TargetPath,
                    settings.TargetWorkingDir,
                    settings.TargetArgs,
                    true,
                    false);

                try
                {
                    if (settings.OutputToFile)
                    {
                        StreamWriter writer = File.CreateText(settings.FileNameForReport);
                        CoverageReportHelper.WriteReport(connector.BlockWalker.Report, writer);
                        writer.Close();
                    }
                    else
                    {
                        CoverageReportHelper.WriteReport(connector.BlockWalker.Report, Console.Out);
                    }

                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine("Can't save report (" + ex.Message + ")");
                }

                if (connector.TargetExitCode.HasValue)
                    return connector.TargetExitCode.Value;
            }
            catch (SettingsException ex)
            {
                Console.Error.WriteLine(ex.Message);
                return -1;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine(ex.Message);
                Console.Error.WriteLine(ex.StackTrace);
                return -1;
            }

            return 0;
        }

        static void ProcessCallback_OnMessage(object sender, EventArgs<CoverageReport.RunHistoryMessage> e)
        {
        }

        static void connector_OnEventMessage(object sender, EventArgs<CoverageReport.RunLogMessage> e)
        {
        }
    }
}

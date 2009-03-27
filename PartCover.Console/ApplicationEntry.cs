using System;
using System.IO;
using System.Xml;
using PartCover.Framework;

namespace PartCover
{
    class ApplicationEntry
    {
        [STAThread]
        static int Main(string[] args)
        {
            try
            {
                var settings = new WorkSettings();
                if (!settings.InitializeFromCommandLine(args))
                {
                    return -1;
                }

                var connector = new Connector();
                connector.StatusMessageReceived += connector_StatusMessageReceived;
                connector.LogEntryReceived += connector_LogEntryReceived;

                connector.UseFileLogging(true);
                connector.UsePipeLogging(false);
                connector.SetLogging((Logging)settings.LogLevel);

                foreach (var item in settings.IncludeItems)
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

                foreach (var item in settings.ExcludeItems)
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
                    XmlWriter writer = settings.OutputToFile
                        ? new XmlTextWriter(File.CreateText(settings.FileNameForReport))
                        : new XmlTextWriter(Console.Out);
                    using (writer)
                    {
                        ReportSerializer.Save(writer, connector.Report);
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

        static void connector_LogEntryReceived(object sender, LogEntryEventArgs e)
        {
            Console.Error.WriteLine(e.Data.ToHumanString());
        }

        static void connector_StatusMessageReceived(object sender, StatusEventArgs e)
        {
            Console.Error.WriteLine(e.Data);
        }
    }
}

using System;
using System.IO;
using System.Xml;
using PartCover.Framework;
using PartCover.Framework.Data;

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

                if (settings.ErrorsToStdout)
                {
                    Console.SetError(Console.Out);
                }

                var connector = new Connector();
                connector.StatusMessageReceived += connector_StatusMessageReceived;
                connector.LogEntryReceived += connector_LogEntryReceived;

                connector.UseFileLogging(true);
                connector.UsePipeLogging(true);
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

                var options = new SessionRunOptions
                {
                    TargetPath = settings.TargetPath,
                    TargetDirectory = settings.TargetWorkingDir,
                    TargetArguments = settings.TargetArgs,
                    RedirectOutput = true,
                    DelayClose = false,
                    FlattenDomains = !settings.DisableFlattenDomains
                };

                connector.Options = options;
                connector.StartTarget();

                try
                {
                    var writer = settings.OutputToFile
                        ? new XmlTextWriter(File.CreateText(settings.FileNameForReport))
                        : new XmlTextWriter(Console.Out);
                    using (writer)
                    {
                        writer.Formatting = Formatting.Indented;
                        writer.Indentation = 1;
                        writer.IndentChar = ' ';
                        ReportSerializer.Save(writer, connector.Report);
                    }
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine("Can't save report (" + ex.Message + ")");
                }

#if DEBUG
                WriteListOfSkippedItems(connector.Report);
#endif

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

        private static void WriteListOfSkippedItems(Report report)
        {
            report.SkippedItems.ForEach(x =>
                Console.Error.WriteLine("Skipped item [{0}]{1}", x.AssemblyName, x.TypedefName));
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

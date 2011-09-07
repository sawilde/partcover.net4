using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Xml;

namespace PartCover.Framework
{
    public class SettingsException : Exception
    {
        public SettingsException(string message) : base(message) { }
    }

    public class WorkSettings
    {
        #region ArgumentOption
        private class ArgumentOption
        {
            public readonly string key;
            public readonly OptionHandler handler;
            public readonly ActivateHandler activator;

            public ArgumentOption(string key, OptionHandler handler)
                : this(key, handler, null)
            {
            }

            public ArgumentOption(string key, ActivateHandler activator)
                : this(key, null, activator)
            {
            }

            private ArgumentOption(string key, OptionHandler handler, ActivateHandler activator)
            {
                this.key = key;
                this.handler = handler;
                this.activator = activator;
            }

            internal delegate void OptionHandler(WorkSettings settings, string value);

            internal delegate void ActivateHandler(WorkSettings settings);
        }

        #endregion ArgumentOption

        private static readonly ArgumentOption[] Options =
        { 
            new ArgumentOption("--register", register),
            new ArgumentOption("--target", readTarget),
            new ArgumentOption("--version", readVersion),
            new ArgumentOption("--help", readHelp),
            new ArgumentOption("--target-work-dir", readTargetWorkDir),
            new ArgumentOption("--generate", readGenerateSettingsFile),
            new ArgumentOption("--log", readLogLevel),
            new ArgumentOption("--target-args", readTargetArgs),
            new ArgumentOption("--include", readInclude),
            new ArgumentOption("--exclude", readExclude),
            new ArgumentOption("--output", readOutput),
            new ArgumentOption("--settings", readSettingsFile),
            new ArgumentOption("--disabledomainflatten", readDisableDomainFlatten),
            new ArgumentOption("--errors-to-stdout", readErrorsToStdOut)
        };

        private ArgumentOption currentOption;

        #region settings readers
        private static void register(WorkSettings settings)
        {
            settings.Register = true;
        }

        private static void readTarget(WorkSettings settings, string value)
        {
            if (!File.Exists(value))
                throw new SettingsException("Cannot find target (" + value + ")");
            settings.TargetPath = Path.GetFullPath(value);
        }

        private static void readGenerateSettingsFile(WorkSettings settings, string value)
        {
            settings.generateSettingsFileName = value;
        }

        private static void readErrorsToStdOut(WorkSettings settings)
        {
            settings.ErrorsToStdout = true;
        }

        private static void readHelp(WorkSettings settings)
        {
            settings.printLongHelp = true;
        }

        private static void readVersion(WorkSettings settings)
        {
            settings.printVersion = true;
        }

        private static void readTargetWorkDir(WorkSettings settings, string value)
        {
            if (!Directory.Exists(value))
                throw new SettingsException("Cannot find target working dir (" + value + ")");
            settings.TargetWorkingDir = Path.GetFullPath(value);
        }

        private static void readSettingsFile(WorkSettings settings, string value)
        {
            if (!File.Exists(value))
                throw new SettingsException("Cannot find settings file (" + value + ")");
            settings.settingsFile = value;
        }

        private static void readOutput(WorkSettings settings, string value)
        {
            if (value.Length > 0) settings.FileNameForReport = Path.GetFullPath(value);
        }

        private static void readExclude(WorkSettings settings, string value)
        {
            if (value.Length > 0) settings.excludeItems.Add(value);
        }

        private static void readInclude(WorkSettings settings, string value)
        {
            if (value.Length > 0) settings.includeItems.Add(value);
        }

        private static void readTargetArgs(WorkSettings settings, string value)
        {
            settings.TargetArgs = value;
        }

        private static void readDisableDomainFlatten(WorkSettings settings)
        {
            settings.DisableFlattenDomains = true;
        }

        private static void readLogLevel(WorkSettings settings, string value)
        {
            try
            {
                settings.logLevel = int.Parse(value, CultureInfo.InvariantCulture);
            }
            catch (Exception ex)
            {
                throw new SettingsException("Wrong value for --log (" + ex.Message + ")");
            }
        }
        #endregion

        #region Parse Args

        private bool printVersion;
        private bool printLongHelp;

        public bool InitializeFromCommandLine(string[] args)
        {
            currentOption = null;

            foreach (string arg in args)
            {
                ArgumentOption nextOption = getOptionHandler(arg);
                if (nextOption != null)
                {
                    currentOption = nextOption;

                    if (currentOption.activator != null)
                        currentOption.activator(this);

                    continue;
                }

                if (currentOption == null)
                {
                    PrintShortUsage(true);
                    return false;
                }

                if (currentOption.handler != null)
                {
                    currentOption.handler(this, arg);
                }
                else
                {
                    throw new SettingsException("Unexpected argument for option '" + currentOption.key + "'");
                }
            }

            if (settingsFile != null)
            {
                ReadSettingsFile();
            }
            else if (generateSettingsFileName != null)
            {
                GenerateSettingsFile();
                return false;
            }
            bool showShort = true;
            if (printLongHelp)
            {
                showShort = false;
                PrintVersion();
                PrintShortUsage(false);
                PrintLongUsage();
            }
            else if (printVersion)
            {
                PrintVersion();
            }

            if (!string.IsNullOrEmpty(TargetPath))
                return true;

            if (showShort)
            {
                PrintShortUsage(true);
            }
            return false;
        }

        private static ArgumentOption getOptionHandler(string arg)
        {
            foreach (ArgumentOption o in Options)
            {
                if (o.key.Equals(arg, StringComparison.CurrentCulture))
                    return o;
            }

            if (arg.StartsWith("--"))
            {
                throw new SettingsException("Invalid option '" + arg + "'");
            }

            return null;
        }

        #endregion //Parse Args

        #region PrintUsage

        public static void PrintShortUsage(bool showNext)
        {
            Console.Out.WriteLine("Usage:");
            Console.Out.WriteLine("  PartCover.exe  [--register] --target <file_name> [--target-work-dir <path>]");
            Console.Out.WriteLine("                [--target-args <arguments>] [--settings <file_name>]");
            Console.Out.WriteLine("                [--include <item> ... ] [--exclude <item> ... ]");
            Console.Out.WriteLine("                [--output <file_name>] [--log <log_level>]");
            Console.Out.WriteLine("                [--generate <file_name>] [--help] [--version]");
            Console.Out.WriteLine("                [--errors-to-stdout");
            Console.Out.WriteLine("");
            if (showNext)
            {
                Console.Out.WriteLine("For more help execute:");
                Console.Out.WriteLine("  PartCover.exe --help");
            }
        }

        public static void PrintLongUsage()
        {
            Console.Out.WriteLine("Arguments:  ");
            Console.Out.WriteLine("   --register :");
            Console.Out.WriteLine("       the COM components are temporarily registered to the current user for profiling,");
            Console.Out.WriteLine("       removing the need to install with elevated privilege.");
            Console.Out.WriteLine("   --target=<file_name> :");
            Console.Out.WriteLine("       specifies path to executable file to count coverage. <file_name> may be");
            Console.Out.WriteLine("       either full path or relative path to file.");
            Console.Out.WriteLine("   --target-work-dir=<path> :");
            Console.Out.WriteLine("       specifies working directory to target process. By default, working");
            Console.Out.WriteLine("       directory will be working directory for PartCover");
            Console.Out.WriteLine("   --target-args=<arguments> :");
            Console.Out.WriteLine("       specifies arguments for target process. If target argument contains");
            Console.Out.WriteLine("       spaces - quote <argument>. If you want specify quote (\") in <arguments>,");
            Console.Out.WriteLine("       then precede it by slash (\\)");
            Console.Out.WriteLine("   --include=<item>, --exclude=<item> :");
            Console.Out.WriteLine("       specifies item to include or exclude from report. Item is in following format: ");
            Console.Out.WriteLine("          [<assembly_regexp>]<class_regexp>");
            Console.Out.WriteLine("       where <regexp> is simple regular expression, containing only asterix and");
            Console.Out.WriteLine("       characters to point item. For example:");
            Console.Out.WriteLine("          [mscorlib]*");
            Console.Out.WriteLine("          [System.*]System.IO.*");
            Console.Out.WriteLine("          [System]System.Colle*");
            Console.Out.WriteLine("          [Test]Test.*+InnerClass+SecondInners*");
            Console.Out.WriteLine("   --settings=<file_name> :");
            Console.Out.WriteLine("       specifies input settins in xml file.");
            Console.Out.WriteLine("   --generate=<file_name> :");
            Console.Out.WriteLine("       generates setting file using settings specified. By default, <file_name>");
            Console.Out.WriteLine("       is 'PartCover.settings.xml'");
            Console.Out.WriteLine("   --output=<file_name> :");
            Console.Out.WriteLine("       specifies output file for writing result xml. It will be placed in UTF-8");
            Console.Out.WriteLine("       encoding. By default, output data will be processed via console output.");
            Console.Out.WriteLine("   --log=<log_level> :");
            Console.Out.WriteLine("       specifies log level for driver. If <log_level> greater than 0, log file");
            Console.Out.WriteLine("       will be created in working directory for PartCover");
            Console.Out.WriteLine("   --help :");
            Console.Out.WriteLine("       shows current help");
            Console.Out.WriteLine("   --version :");
            Console.Out.WriteLine("       shows version of PartCover console application");
            Console.Out.WriteLine("   --errors-to-stdout :");
            Console.Out.WriteLine("       errors are sent to the standard output stream instead of the the error output stream");
            Console.Out.WriteLine("");
        }

        public static void PrintVersion()
        {
            Console.Out.WriteLine("PartCover (console)");
            Console.Out.WriteLine("   application version {0}.{1}.{2}",
                Assembly.GetExecutingAssembly().GetName().Version.Major,
                Assembly.GetExecutingAssembly().GetName().Version.Minor,
                Assembly.GetExecutingAssembly().GetName().Version.Revision);
            Type connector = typeof(Connector);
            Console.Out.WriteLine("   connector version {0}.{1}.{2}",
                connector.Assembly.GetName().Version.Major,
                connector.Assembly.GetName().Version.Minor,
                connector.Assembly.GetName().Version.Revision);
            Console.Out.WriteLine("");
        }

        #endregion PrintUsage

        #region Properties

        private string settingsFile;
        public string SettingsFile
        {
            get { return settingsFile; }
            set { settingsFile = value; }
        }

        private string generateSettingsFileName;
        public string GenerateSettingsFileName
        {
            get { return generateSettingsFileName; }
            set { generateSettingsFileName = value; }
        }

        private int logLevel;
        public int LogLevel
        {
            get { return logLevel; }
            set { logLevel = value; }
        }

        private readonly List<string> includeItems = new List<string>();
        public string[] IncludeItems
        {
            get { return includeItems.ToArray(); }
        }

        private readonly List<string> excludeItems = new List<string>();
        public string[] ExcludeItems
        {
            get { return excludeItems.ToArray(); }
        }

        public string TargetPath { get; set; }
        public string TargetWorkingDir { get; set; }
        public string TargetArgs { get; set; }
        public string FileNameForReport { get; set; }

        public bool ErrorsToStdout { get; set; }

        public bool DisableFlattenDomains { get; set; }

        public bool OutputToFile
        {
            get { return FileNameForReport != null; }
        }

        public bool Register { get; set; }

        #endregion //Properties

        #region SerializeSettings

        private static void AppendValue(XmlNode parent, string name, string value)
        {
            Debug.Assert(parent != null && parent.OwnerDocument != null);
            parent.AppendChild(parent.OwnerDocument.CreateElement(name)).InnerText = value;
        }

        public void GenerateSettingsFile()
        {
            var xmlDoc = new XmlDocument();
            xmlDoc.AppendChild(xmlDoc.CreateElement("PartCoverSettings"));
            if (TargetPath != null) AppendValue(xmlDoc.DocumentElement, "Target", TargetPath);
            if (TargetWorkingDir != null) AppendValue(xmlDoc.DocumentElement, "TargetWorkDir", TargetWorkingDir);
            if (TargetArgs != null) AppendValue(xmlDoc.DocumentElement, "TargetArgs", TargetArgs);
            if (logLevel > 0) AppendValue(xmlDoc.DocumentElement, "LogLevel", logLevel.ToString(CultureInfo.InvariantCulture));
            if (FileNameForReport != null) AppendValue(xmlDoc.DocumentElement, "Output", FileNameForReport);
            if (DisableFlattenDomains) AppendValue(xmlDoc.DocumentElement, "DisableFlattenDomains", DisableFlattenDomains.ToString(CultureInfo.InvariantCulture));
            if (printLongHelp) AppendValue(xmlDoc.DocumentElement, "ShowHelp", printLongHelp.ToString(CultureInfo.InvariantCulture));
            if (printVersion) AppendValue(xmlDoc.DocumentElement, "ShowVersion", printVersion.ToString(CultureInfo.InvariantCulture));
            if (ErrorsToStdout) AppendValue(xmlDoc.DocumentElement, "ErrorsToStdout", ErrorsToStdout.ToString(CultureInfo.InvariantCulture));

            foreach (var item in IncludeItems) AppendValue(xmlDoc.DocumentElement, "Rule", "+" + item);
            foreach (var item in ExcludeItems) AppendValue(xmlDoc.DocumentElement, "Rule", "-" + item);

            try
            {
                if ("console".Equals(generateSettingsFileName, StringComparison.InvariantCulture))
                    xmlDoc.Save(Console.Out);
                else
                    xmlDoc.Save(generateSettingsFileName);
            }
            catch (Exception ex)
            {
                throw new SettingsException("Cannot write settings (" + ex.Message + ")");
            }
        }

        public void ReadSettingsFile()
        {
            try
            {
                var xmlDoc = new XmlDocument();
                var settingsFolder = Path.GetDirectoryName(settingsFile) ?? string.Empty;
                xmlDoc.Load(settingsFile);
                logLevel = 0;

                var node = xmlDoc.SelectSingleNode("/PartCoverSettings/Target/text()");
                if (node != null && node.Value != null) TargetPath = Path.GetFullPath(Path.Combine(settingsFolder, node.Value));
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/TargetWorkDir/text()");
                if (node != null && node.Value != null) TargetWorkingDir = Path.GetFullPath(Path.Combine(settingsFolder, node.Value));
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/TargetArgs/text()");
                if (node != null && node.Value != null) TargetArgs = node.Value;
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/LogLevel/text()");
                if (node != null && node.Value != null) logLevel = int.Parse(node.Value);
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/Output/text()");
                if (node != null && node.Value != null) FileNameForReport =  Path.GetFullPath(Path.Combine(settingsFolder, node.Value));
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/ShowHelp/text()");
                if (node != null && node.Value != null) printLongHelp = bool.Parse(node.Value);
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/DisableFlattenDomains/text()");
                if (node != null && node.Value != null) DisableFlattenDomains = bool.Parse(node.Value);
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/ShowVersion/text()");
                if (node != null && node.Value != null) printVersion = bool.Parse(node.Value);
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/ErrorsToStdout/text()");
                if (node != null && node.Value != null) ErrorsToStdout = bool.Parse(node.Value);

                var list = xmlDoc.SelectNodes("/PartCoverSettings/Rule");
                if (list != null)
                {
                    foreach (XmlNode rule in list)
                    {
                        var ruleText = rule.SelectSingleNode("text()");
                        if (ruleText == null || string.IsNullOrEmpty(ruleText.Value))
                            continue;

                        var rules = ruleText.Value.Split(',');
                        foreach (var s in rules)
                        {
                            if (s.Length <= 1)
                                continue;
                            switch (s[0])
                            {
                                case '+':
                                    includeItems.Add(s.Substring(1));
                                    break;
                                case '-':
                                    excludeItems.Add(s.Substring(1));
                                    break;
                                default:
                                    throw new SettingsException("Wrong rule format (" + s + ")");
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                throw new SettingsException("Cannot load settings (" + ex.Message + ")");
            }
        }

        #endregion

        public void IncludeRules(ICollection<string> strings)
        {
            includeItems.AddRange(strings);
        }

        public void ExcludeRules(ICollection<string> strings)
        {
            excludeItems.AddRange(strings);
        }

        public void ClearRules()
        {
            includeItems.Clear();
            excludeItems.Clear();
        }
    }
}

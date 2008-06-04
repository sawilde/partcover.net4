using System;
using System.Xml;
using System.Diagnostics;
using System.Globalization;
using System.Reflection;
using System.IO;
using System.Collections;

namespace PartCover.Framework
{
    public class SettingsException : Exception {
        public SettingsException(string message) : base(message) {}
    }

	public class WorkSettings
	{
		public WorkSettings() {}

        #region Parse Args 

        public void ExtractArgument(string str, out string argument, out string argumentValue) {
            int argParamIndex = str.IndexOf('=');
            if (argParamIndex == -1) {
                argument = str;
                argumentValue = string.Empty;
            }  else {
                argument = str.Substring(0, argParamIndex);
                if (argParamIndex == str.Length - 1)
                    argumentValue = string.Empty;
                else
                    argumentValue = str.Substring(argParamIndex + 1, str.Length - argParamIndex - 1);
            }
        }

        private bool printVersion = false;
        private bool printLongHelp = false;

        public bool InitializeFromArgs(string[] args) {
            foreach(string arg in args) {
                string argument;
                string argumentValue;
                ExtractArgument(arg, out argument, out argumentValue);
                ProcessArgument(argument, argumentValue);
            }
            if (settingsFile != null) {
                ReadSettingsFile();
            } else if (generateSettingsFileName != null) {
                GenerateSettingsFile();
                return false;
            } 
            bool showShort = true;
            if (printLongHelp) {
                showShort = false;
                PrintVersion();
                PrintShortUsage(false);
                PrintLongUsage();
            } else if (printVersion) {
                PrintVersion();
            }

            if (TargetPath != null && TargetPath.Length > 0)
                return true;
            if (showShort) {
                PrintShortUsage(true);
            }
            return false;
        }

        public void ProcessArgument(string value, string valueArg) {
            switch(value) {
                case "--version" :  printVersion = true; break;
                case "--help" :  printLongHelp = true; break;
                case "--target": {
                    if (!File.Exists(valueArg))
                        throw new SettingsException("Cannot find target");
                    targetPath = Path.GetFullPath(valueArg);
                } break;
                case "--target-work-dir": {
                    if (!Directory.Exists(valueArg))
                        throw new SettingsException("Cannot find target working dir");
                    targetWorkingDir = Path.GetFullPath(valueArg);
                } break;
                case "--generate": {
                    generateSettingsFileName = valueArg;
                } break;
                case "--log": {
                    try {
                        logLevel = int.Parse(valueArg, CultureInfo.InvariantCulture);
                    } catch(Exception ex) {
                        throw new SettingsException("Wrong value for --log (" + ex.Message + ")");
                    }
                } break;
                case "--target-args": {
                    targetArgs = valueArg;
                } break;
                case "--include": {
                    if (valueArg.Length > 0) includeItems.Add(valueArg);
                } break;
                case "--exclude": {
                    if (valueArg.Length > 0) excludeItems.Add(valueArg);
                } break;
                case "--output": {
                    if (valueArg.Length > 0)
                        outputFile = valueArg;
                } break;
                case "--settings": {
                    if (!File.Exists(valueArg)) 
                        throw new SettingsException("Cannot find settings file");
                    settingsFile = valueArg;
                } break;
                default:
                    throw new SettingsException("Unknown option " + value);
            }
        }

        #endregion //Parse Args 

        #region PrintUsage 

        public static void PrintShortUsage(bool showNext) {
            System.Console.Out.WriteLine("Usage:");
            System.Console.Out.WriteLine("  PartCover.exe  --target=<file_name> [--target-work-dir=<path>]");
            System.Console.Out.WriteLine("                [--target-args=<arguments>] [--settings=<file_name>]");
            System.Console.Out.WriteLine("                [--include=<item>] [--exclude=<item>]");
            System.Console.Out.WriteLine("                [--output=<file_name>] [--log=<log_level>]");
            System.Console.Out.WriteLine("                [--generate=<file_name>] [--help] [--version]");
            System.Console.Out.WriteLine("");
            if (showNext) {
                System.Console.Out.WriteLine("For more help execute:");
                System.Console.Out.WriteLine("  PartCover.exe --help");
            }
        }

        public static void PrintLongUsage() {
            System.Console.Out.WriteLine("Arguments:  ");
            System.Console.Out.WriteLine("   --target=<file_name> :");
            System.Console.Out.WriteLine("       specifies path to executable file to count coverage. <file_name> may be");
            System.Console.Out.WriteLine("       either full path or relative path to file.");
            System.Console.Out.WriteLine("   --target-work-dir=<path> :");
            System.Console.Out.WriteLine("       specifies working directory to target process. By default, working");
            System.Console.Out.WriteLine("       directory will be working directory for PartCover");
            System.Console.Out.WriteLine("   --target-args=<arguments> :");
            System.Console.Out.WriteLine("       specifies arguments for target process. If target argument contains");
            System.Console.Out.WriteLine("       spaces - quote <argument>. If you want specify quote (\") in <arguments>,");
            System.Console.Out.WriteLine("       then precede it by slash (\\)");
            System.Console.Out.WriteLine("   --include=<item>, --exclude=<item> :");
            System.Console.Out.WriteLine("       specifies item to include or exclude from report. Item is in following format: ");
            System.Console.Out.WriteLine("          [<assembly_regexp>]<class_regexp>");
            System.Console.Out.WriteLine("       where <regexp> is simple regular expression, containing only asterix and");
            System.Console.Out.WriteLine("       characters to point item. For example:");
            System.Console.Out.WriteLine("          [mscorlib]*");
            System.Console.Out.WriteLine("          [System.*]System.IO.*");
            System.Console.Out.WriteLine("          [System]System.Colle*");
            System.Console.Out.WriteLine("          [Test]Test.*+InnerClass+SecondInners*");
            System.Console.Out.WriteLine("   --settings=<file_name> :");
            System.Console.Out.WriteLine("       specifies input settins in xml file.");
            System.Console.Out.WriteLine("   --generate=<file_name> :");
            System.Console.Out.WriteLine("       generates setting file using settings specified. By default, <file_name>");
            System.Console.Out.WriteLine("       is 'PartCover.settings.xml'");
            System.Console.Out.WriteLine("   --output=<file_name> :");
            System.Console.Out.WriteLine("       specifies output file for writing result xml. It will be placed in UTF-8");
            System.Console.Out.WriteLine("       encoding. By default, output data will be processed via console output.");
            System.Console.Out.WriteLine("   --log=<log_level> :");
            System.Console.Out.WriteLine("       specifies log level for driver. If <log_level> greater than 0, log file");
            System.Console.Out.WriteLine("       will be created in working directory for PartCover");
            System.Console.Out.WriteLine("   --help :");
            System.Console.Out.WriteLine("       shows current help");
            System.Console.Out.WriteLine("   --version :");
            System.Console.Out.WriteLine("       shows version of PartCover console application");
            System.Console.Out.WriteLine("");
        }

        public static void PrintVersion() {
            System.Console.Out.WriteLine("PartCover (console)");
            System.Console.Out.WriteLine("   application version {0}.{1}.{2}", 
                Assembly.GetExecutingAssembly().GetName().Version.Major,
                Assembly.GetExecutingAssembly().GetName().Version.Minor,
                Assembly.GetExecutingAssembly().GetName().Version.Revision);
            Type connector = typeof(PartCover.Framework.Connector);
            System.Console.Out.WriteLine("   connector version {0}.{1}.{2}", 
                connector.Assembly.GetName().Version.Major,
                connector.Assembly.GetName().Version.Minor,
                connector.Assembly.GetName().Version.Revision);
            System.Console.Out.WriteLine("");
        }

        #endregion PrintUsage 

        #region Properties

        private string settingsFile;
        public string SettingsFile {
            get { return settingsFile; }
            set { settingsFile = value; }
        }

        private string generateSettingsFileName;
        public string GenerateSettingsFileName {
            get { return generateSettingsFileName; }
            set { generateSettingsFileName = value; }
        }

        private int logLevel = 0;
        public int LogLevel {
            get { return logLevel; }
        }

        private ArrayList includeItems = new ArrayList();
        public string[] IncludeItems {
            get { 
                return includeItems.Count == 0
                    ? new string[0] 
                    : (string[]) includeItems.ToArray(typeof(String));
            }
        }

        private ArrayList excludeItems = new ArrayList();
        public string[] ExcludeItems {
            get { 
                return excludeItems.Count == 0
                    ? new string[0] 
                    : (string[]) excludeItems.ToArray(typeof(String));
            }
        }

        private string targetPath;
        public string TargetPath {
            get { return targetPath; }
            set { targetPath = value; }
        }

        private string targetWorkingDir;
        public string TargetWorkingDir {
            get { return targetWorkingDir; }
            set { targetWorkingDir = value; }
        }

        private string targetArgs = null;
        public string TargetArgs {
            get { return targetArgs; }
            set { targetArgs = value; }
        }

        private string outputFile = null;
        public string FileNameForReport {
            get { return outputFile; }
            set { outputFile = value; }
        }

        public bool OutputToFile {
            get { return FileNameForReport != null; }
        }

        #endregion //Properties

        #region SerializeSettings

        private void AppendValue(XmlNode parent, string name, string value) {
            Debug.Assert(parent != null && parent.OwnerDocument != null);
            XmlNode node = parent.AppendChild(parent.OwnerDocument.CreateElement(name));
            node.InnerText = value;
        }

        public void GenerateSettingsFile() {
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.AppendChild(xmlDoc.CreateElement("PartCoverSettings"));
            if (targetPath != null) AppendValue(xmlDoc.DocumentElement, "Target", targetPath);
            if (targetWorkingDir != null) AppendValue(xmlDoc.DocumentElement, "TargetWorkDir", targetWorkingDir);
            if (targetArgs != null) AppendValue(xmlDoc.DocumentElement, "TargetArgs", targetArgs);
            if (logLevel > 0) AppendValue(xmlDoc.DocumentElement, "LogLevel", logLevel.ToString(CultureInfo.InvariantCulture));
            if (outputFile != null) AppendValue(xmlDoc.DocumentElement, "Output", outputFile);
            if (printLongHelp) AppendValue(xmlDoc.DocumentElement, "ShowHelp", printLongHelp.ToString(CultureInfo.InvariantCulture));
            if (printVersion) AppendValue(xmlDoc.DocumentElement, "ShowVersion", printVersion.ToString(CultureInfo.InvariantCulture));

            foreach(string item in IncludeItems) AppendValue(xmlDoc.DocumentElement, "Rule", "+" + item);
            foreach(string item in ExcludeItems) AppendValue(xmlDoc.DocumentElement, "Rule", "-" + item);

            try {
                if (generateSettingsFileName != null && generateSettingsFileName.Length > 0)
                    xmlDoc.Save(generateSettingsFileName);
                else 
                    xmlDoc.Save(System.Console.Out);
            } catch (Exception ex) {
                throw new SettingsException("Cannot write settings (" + ex.Message + ")");
            }
        }

        public void ReadSettingsFile() {
            XmlDocument xmlDoc = new XmlDocument();
            try {
                xmlDoc.Load(settingsFile);

                XmlNode node = xmlDoc.SelectSingleNode("/PartCoverSettings/Target/text()");
                if (node != null && node.Value != null) targetPath = node.Value;
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/TargetWorkDir/text()");
                if (node != null && node.Value != null) targetWorkingDir = node.Value;
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/TargetArgs/text()");
                if (node != null && node.Value != null) targetArgs = node.Value;
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/LogLevel/text()");
                if (node != null && node.Value != null) logLevel = int.Parse(node.Value);
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/Output/text()");
                if (node != null && node.Value != null) outputFile = node.Value;
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/ShowHelp/text()");
                if (node != null && node.Value != null) printLongHelp = bool.Parse(node.Value);
                node = xmlDoc.SelectSingleNode("/PartCoverSettings/ShowVersion/text()");
                if (node != null && node.Value != null) printVersion = bool.Parse(node.Value);

                XmlNodeList list = xmlDoc.SelectNodes("/PartCoverSettings/Rule");
                if (list != null) {
                    foreach(XmlNode rule in list) {
                        XmlNode ruleText = rule.SelectSingleNode("text()");
                        if (ruleText == null || ruleText.Value == null || ruleText.Value.Length == 0) 
                            continue;
                        string[] rules = ruleText.Value.Split(',');
                        foreach(string s in rules) {
                            if (s.Length <= 1)
                                continue;
                            if (s[0] == '+')
                                includeItems.Add(s.Substring(1));
                            else if (s[0] == '-')
                                excludeItems.Add(s.Substring(1));
                            else 
                                throw new SettingsException("Wrong rule format (" + s + ")");
                        }
                    }
                }
            } catch(Exception ex) {
                throw new SettingsException("Cannot load settings (" + ex.Message + ")");
            }
        }

        #endregion

        public void IncludeRules(ICollection strings) {
            includeItems.AddRange(strings);
        }

        public void ExcludeRules(ICollection strings) {
            excludeItems.AddRange(strings);
        }

        public void ClearRules() {
            includeItems.Clear();
            excludeItems.Clear();
        }
    }
}

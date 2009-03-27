using System;
using System.IO;
using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public partial class Connector
    {
        readonly PartCoverConnector2Class connector = new PartCoverConnector2Class();
        readonly ReportReceiver receiver = new ReportReceiver();
        readonly ConnectorActionCallback actionCallback;

        EventHandler<StatusEventArgs> statusMessageReceived;
        public event EventHandler<StatusEventArgs> StatusMessageReceived
        {
            add { statusMessageReceived += value; }
            remove { statusMessageReceived -= value; }
        }

        EventHandler<LogEntryEventArgs> logEntryReceived;
        public event EventHandler<LogEntryEventArgs> LogEntryReceived
        {
            add { logEntryReceived += value; }
            remove { logEntryReceived -= value; }
        }

        public Connector()
        {
            actionCallback = new ConnectorActionCallback(this);
        }

        public Report Report { get { return receiver.Report; } }

        public void StartTarget(string path, string directory, string args, bool redirectOutput, bool delayClose)
        {
            // set mode
            connector.EnableOption(ProfilerMode.COUNT_COVERAGE);

            ExcludeItem("[mscorlib]*");
            ExcludeItem("[System*]*");

            if (directory != null)
            {
                directory = directory.Trim();
            }
            if (path != null)
            {
                path = path.Trim();
            }
            if (args != null)
            {
                args = args.Trim();
            }
            if (string.IsNullOrEmpty(directory))
            {
                directory = Directory.GetCurrentDirectory();
            }

            // start target
            //  ProcessCallback.writeStatus("Start target");
            connector.StartTarget(path, directory, args, redirectOutput, actionCallback);

            // wait results
            //ProcessCallback.writeStatus("Wait results");
            connector.WaitForResults(delayClose, actionCallback);

            // walk results
            //ProcessCallback.writeStatus("Walk results");
            receiver.Report = new Report();
            connector.GetReport(receiver);
        }

        public void CloseTarget()
        {
            connector.CloseTarget();
        }

        public int? TargetExitCode
        {
            get
            {
                if (connector != null && connector.HasTargetExitCode)
                    return connector.TargetExitCode;
                return null;
            }
        }

        public int TargetProcessId
        {
            get
            {
                if (connector == null)
                    throw new InvalidOperationException("No connector available");
                return connector.ProcessId;
            }
        }

        public string DriverLogFile
        {
            get
            {
                if (connector == null)
                    throw new InvalidOperationException("No connector available");
                return connector.LogFilePath;
            }
        }

        internal void OnLogMessage(LogEntry message)
        {
            var eventHandler = logEntryReceived;
            if (eventHandler == null) { return; }
            eventHandler.Invoke(this, new LogEntryEventArgs(message));
        }

        internal void OnStatusReceive(string message)
        {
            var eventHandler = statusMessageReceived;
            if (eventHandler == null) { return; }
            eventHandler.Invoke(this, new StatusEventArgs(message));
        }
    }
}

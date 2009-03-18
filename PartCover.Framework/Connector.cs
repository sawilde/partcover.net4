using System;
using System.IO;
using PartCover.Framework.Walkers;

namespace PartCover.Framework
{
    public class Connector
    {
        readonly PartCoverConnector2Class connector = new PartCoverConnector2Class();

        public Connector()
        {
            ActionCallback = new ConnectorActionCallback(this);
            ProcessCallback = new ProcessCallback();
            ProcessCallback.OnMessage += processCallbackOnMessage;
        }

        public event EventHandler<EventArgs<CoverageReport.RunLogMessage>> OnEventMessage;

        public ProcessCallback ProcessCallback { get; set; }

        internal ConnectorActionCallback ActionCallback { get; set; }

        public void SetLogging(Logging logging)
        {
            connector.LoggingLevel = (int)logging;
        }

        public void UseFileLogging(bool logging)
        {
            connector.FileLoggingEnable = logging;
        }

        public void UsePipeLogging(bool logging)
        {
            connector.PipeLoggingEnable = logging;
        }

        private InstrumentedBlocksWalkerInner blockWalker;
        public InstrumentedBlocksWalker BlockWalker
        {
            get { return blockWalker; }
        }

        public void StartTarget(string path, string directory, string args, bool redirectOutput, bool delayClose)
        {
            blockWalker = new InstrumentedBlocksWalkerInner();

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
            ProcessCallback.writeStatus("Start target");
            connector.StartTarget(path, directory, args, redirectOutput, ActionCallback);

            // wait results
            ProcessCallback.writeStatus("Wait results");
            connector.WaitForResults(delayClose, ActionCallback);

            // walk results
            ProcessCallback.writeStatus("Walk results");
            connector.WalkInstrumentedResults(blockWalker);

            if (connector.HasTargetExitCode) blockWalker.Report.ExitCode = connector.TargetExitCode;
        }

        internal void OnLogMessage(CoverageReport.RunLogMessage message)
        {
            blockWalker.Report.runLog.Add(message);
            if (OnEventMessage != null) OnEventMessage(this, new EventArgs<CoverageReport.RunLogMessage>(message));
        }

        private void processCallbackOnMessage(object sender, EventArgs<CoverageReport.RunHistoryMessage> e)
        {
            blockWalker.Report.runHistory.Add(e.Data);
        }

        public void CloseTarget()
        {
            connector.CloseTarget();
        }

        public void IncludeItem(string item)
        {
            connector.IncludeItem(item);
        }

        public void ExcludeItem(string item)
        {
            connector.ExcludeItem(item);
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

    }
}

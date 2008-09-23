using System;
using System.IO;
using PartCover.Framework.Walkers;

namespace PartCover.Framework
{
    public class Connector
    {
        readonly PartCoverConnector2Class connector = new PartCoverConnector2Class();

        public Connector() { }

        public event Action<Connector> AfterStart;

        private IProgressCallback outWriter;
        public IProgressCallback Out
        {
            set { outWriter = value; }
            get { return outWriter ?? new ProgressCallbackStub(); }
        }

        private IConnectorActionCallback actionWriter;
        public IConnectorActionCallback ActionCallback
        {
            set { actionWriter = value; }
            get { return actionWriter; }
        }

        public void SetLogging(Logging logging)
        {
            connector.SetVerbose((int)logging);
        }

        private readonly InstrumentedBlocksWalkerInner blockWalker = new InstrumentedBlocksWalkerInner();
        public InstrumentedBlocksWalker BlockWalker
        {
            get { return blockWalker; }
        }

        public void StartTarget(string path, string directory, string args, bool redirectOutput, bool delayClose)
        {
            // set mode
            connector.EnableOption(ProfilerMode.COUNT_COVERAGE);

            ExcludeItem("[mscorlib]*");
            ExcludeItem("[System*]*");

            if (directory != null) directory = directory.Trim();
            if (path != null) path = path.Trim();
            if (args != null) args = args.Trim();

            if (directory == null || directory.Length == 0)
                directory = Directory.GetCurrentDirectory();

            // start target
            Out.writeStatus("Start target");
            connector.StartTarget(path, directory, args, redirectOutput, new ActionCallbackStub(ActionCallback));

            if (AfterStart != null) AfterStart(this);

            // wait results
            Out.writeStatus("Wait results");
            connector.WaitForResults(delayClose, new ActionCallbackStub(ActionCallback));

            // walk results
            Out.writeStatus("Walk results");
            connector.WalkInstrumentedResults(blockWalker);
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

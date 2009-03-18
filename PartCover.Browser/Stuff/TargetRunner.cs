using PartCover.Browser.Helpers;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api;
using PartCover.Framework;
using PartCover.Browser.Forms;

namespace PartCover.Browser.Stuff
{
    internal interface IRunTargetProgressTracker : IProgressTracker
    {
        void add(CoverageReport.RunLogMessage runLogMessage);
        void add(CoverageReport.RunHistoryMessage runHistoryMessage);
    }

    internal class TargetRunner : AsyncUserProcess<RunTargetTracker, IRunTargetProgressTracker>
    {
        private RunTargetForm runTargetForm;
        public RunTargetForm RunTargetForm
        {
            get { return runTargetForm; }
            set { runTargetForm = value; }
        }

        CoverageReport report;
        public CoverageReport Report
        {
            get { return report; }
        }

        protected override void doWork()
        {
            var connector = new Connector();
            connector.ProcessCallback.OnMessage += connectorOnMessage;
            connector.OnEventMessage += connectorOnEventMessage;

            Tracker.AppendMessage("Create connector");

            if (runTargetForm.InvokeRequired)
            {
                runTargetForm.Invoke(new InitializeConnectorDelegate(InitializeConnector), connector);
            }
            else
            {
                InitializeConnector(connector);
            }


            Tracker.AppendMessage("Store report");
            report = connector.BlockWalker.Report;

            Tracker.AppendMessage("Done");
            connector.OnEventMessage -= connectorOnEventMessage;
            connector.ProcessCallback.OnMessage -= connectorOnMessage;
        }

        delegate void InitializeConnectorDelegate(Connector connector);
        private void InitializeConnector(Connector connector)
        {
            foreach (var s in runTargetForm.IncludeItems) connector.IncludeItem(s);
            foreach (var s in runTargetForm.ExcludeItems) connector.ExcludeItem(s);

            connector.SetLogging(runTargetForm.LogLevel);
            connector.UseFileLogging(false);
            connector.UsePipeLogging(true);
            connector.StartTarget(
                runTargetForm.TargetPath,
                runTargetForm.TargetWorkingDir,
                runTargetForm.TargetArgs,
                false, false);
        }

        void connectorOnEventMessage(object sender, EventArgs<CoverageReport.RunLogMessage> e)
        {
            Tracker.add(e.Data);
        }

        void connectorOnMessage(object sender, EventArgs<CoverageReport.RunHistoryMessage> e)
        {
            Tracker.add(e.Data);
        }
    }
}

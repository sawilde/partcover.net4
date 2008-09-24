using PartCover.Browser.Dialogs;
using PartCover.Browser.Helpers;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api;
using System;
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
            Framework.Connector connector = new Framework.Connector();
            connector.ProcessCallback.OnMessage += connectorOnMessage;
            connector.OnEventMessage += connectorOnEventMessage;

            Tracker.setMessage("Create connector");

            if (runTargetForm.InvokeRequired)
            {
                runTargetForm.Invoke(new InitializeConnectorDelegate(InitializeConnector), connector);
            }
            else
            {
                InitializeConnector(connector);
            }


            Tracker.setMessage("Store report");
            report = connector.BlockWalker.Report;

            Tracker.setMessage("Done");
            connector.OnEventMessage -= connectorOnEventMessage;
            connector.ProcessCallback.OnMessage -= connectorOnMessage;
        }

        delegate void InitializeConnectorDelegate(Connector connector);
        private void InitializeConnector(Connector connector)
        {
            foreach (string s in runTargetForm.IncludeItems) connector.IncludeItem(s);
            foreach (string s in runTargetForm.ExcludeItems) connector.ExcludeItem(s);

            connector.SetLogging(runTargetForm.LogLevel);
            connector.UseFileLogging(false);
            connector.UsePipeLogging(true);
            connector.StartTarget(
                runTargetForm.TargetPath,
                runTargetForm.TargetWorkingDir,
                runTargetForm.TargetArgs,
                false, false);
        }

        private event EventHandler<EventArgs<CoverageReport.RunHistoryMessage>> OnMessage;
        private event EventHandler<EventArgs<CoverageReport.RunLogMessage>> OnEventMessage;

        void connectorOnEventMessage(object sender, EventArgs<CoverageReport.RunLogMessage> e)
        {
            if (OnEventMessage != null) OnEventMessage(this, e);
            Tracker.add(e.Data);
        }

        void connectorOnMessage(object sender, EventArgs<CoverageReport.RunHistoryMessage> e)
        {
            if (OnMessage != null) OnMessage(this, e);
            Tracker.add(e.Data);
        }
    }
}

using PartCover.Browser.Helpers;
using PartCover.Framework.Data;
using PartCover.Framework;
using PartCover.Browser.Forms;

namespace PartCover.Browser.Stuff
{
    internal interface ITargetProgressTracker
    {
        void ShowStatus(string data);
        void ShowLogMessage(string data);
    }

    internal class TargetRunner : AsyncUserProcess<RunTargetTracker, ITargetProgressTracker>
    {
        private RunTargetForm runTargetForm;
        public RunTargetForm RunTargetForm
        {
            get { return runTargetForm; }
            set { runTargetForm = value; }
        }

        Report report;
        public Report Report
        {
            get { return report; }
        }

        protected override void doWork()
        {
            var connector = new Connector();
            connector.StatusMessageReceived += connector_StatusMessageReceived;
            connector.LogEntryReceived += connector_LogEntryReceived;

            Tracker.ShowStatus("Create connector");

            if (runTargetForm.InvokeRequired)
            {
                runTargetForm.Invoke(new InitializeConnectorDelegate(InitializeConnector), connector);
            }
            else
            {
                InitializeConnector(connector);
            }


            Tracker.ShowStatus("Store report");
            report = connector.Report;

            Tracker.ShowStatus("Done");
            connector.StatusMessageReceived -= connector_StatusMessageReceived;
            connector.LogEntryReceived -= connector_LogEntryReceived;
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

        void connector_StatusMessageReceived(object sender, StatusEventArgs e)
        {
            Tracker.ShowStatus(e.Data);
        }

        void connector_LogEntryReceived(object sender, LogEntryEventArgs e)
        {
            Tracker.ShowLogMessage(e.Data.ToHumanString());
        }
    }
}

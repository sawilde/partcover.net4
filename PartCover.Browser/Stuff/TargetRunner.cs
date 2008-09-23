using PartCover.Browser.Dialogs;
using PartCover.Browser.Helpers;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api;
using System;

namespace PartCover.Browser.Stuff
{
    internal class TargetRunner : AsyncUserProcess<SmallAsyncUserForm>
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

        IRunHistory runHistory;
        public IRunHistory RunHistory
        {
            get { return runHistory; }
            set { runHistory = value; }
        }

        protected override void doWork()
        {
            Callback.writeStatus("Create connector");

            Framework.Connector connector = new Framework.Connector();
            connector.Out = new HistoryCollector(Callback, runHistory);
            connector.ActionCallback = new TargetRunnerCallback(connector.Out);

            foreach (string s in runTargetForm.IncludeItems) connector.IncludeItem(s);
            foreach (string s in runTargetForm.ExcludeItems) connector.ExcludeItem(s);

            connector.AfterStart += new Action<Framework.Connector>(connector_AfterStart);

            connector.SetLogging(runTargetForm.LogLevel);
            connector.StartTarget(
                runTargetForm.TargetPath,
                runTargetForm.TargetWorkingDir,
                runTargetForm.TargetArgs, 
                false, false);

            Callback.writeStatus("Store report");
            report = connector.BlockWalker.Report;

            Callback.writeStatus("Done");
            runHistory.ExitCode = connector.TargetExitCode;
        }

        private void connector_AfterStart(Framework.Connector connector)
        {
            runHistory.DriverLog = connector.DriverLogFile;
        }
    }
}

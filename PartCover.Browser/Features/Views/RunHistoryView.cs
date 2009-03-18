using System;
using System.Windows.Forms;
using PartCover.Browser.Api;
using PartCover.Framework.Walkers;
using System.Globalization;

namespace PartCover.Browser.Features.Views
{
    public partial class RunHistoryView : ReportView
    {
        public RunHistoryView()
        {
            InitializeComponent();
            lbItems.DisplayMember = "Text";
        }

        public override void attach(IServiceContainer container, IProgressTracker tracker)
        {
            base.attach(container, tracker);
            tracker.AppendMessage("Load history data for view");
            setData(Services.getService<ICoverageReportService>().Report, tracker);
        }

        public override void detach(IServiceContainer container, IProgressTracker tracker)
        {
            tracker.AppendMessage("Unload history data");
            removeItems();
            base.detach(container, tracker);
        }

        private struct Counter
        {
            private readonly int step;
            private int current;

            public Counter(int totalCount)
            {
                step = totalCount / 10;
                step = step == 0 ? 1 : step;
                current = 0;
            }

            public bool next() { current++; return atPoint(); }
            private bool atPoint() { return 0 == current % step; }
        }

        private void setData(ICoverageReport report, IProgressTracker tracker)
        {
            if (report == null)
            {
                removeItems();
                return;
            }

            setExitCode(report.ExitCode);

            var history = report.RunHistory;
            var counter = new Counter(history.Count);

            tracker.QueueBegin("Load run history ");
            foreach (var item in history)
            {
                addHistoryItem(new HistoryItemWrapper(item));
                if (counter.next())
                {
                    tracker.QueuePush(".");
                }
            }
            tracker.QueueEnd(string.Empty);

            var log = report.LogEvents;
            counter = new Counter(log.Count);

            tracker.QueueBegin("Load log history ");
            foreach (var item in log)
            {
                addLogItem(item);
                if (counter.next())
                {
                    tracker.QueuePush(".");
                }
            }
            tracker.QueueEnd(string.Empty);
        }

        private delegate void AddHistoryItem(HistoryItemWrapper historyItemWrapper);
        private void addHistoryItem(HistoryItemWrapper historyItemWrapper)
        {
            if (InvokeRequired)
            {
                Invoke(new AddHistoryItem(addHistoryItem), historyItemWrapper);
                return;
            }

            lbItems.Items.Add(historyItemWrapper);
        }

        private delegate void AddLogItemDelegate(CoverageReport.RunLogMessage item);
        private void addLogItem(CoverageReport.RunLogMessage item)
        {
            if (InvokeRequired)
            {
                Invoke(new AddLogItemDelegate(addLogItem), item);
                return;
            }

            tbLog.AppendText(string.Format(CultureInfo.CurrentCulture,
                 "[{0,6}][{1,6}]{2}{3}",
                 item.ThreadId, item.MsOffset, item.Message, Environment.NewLine));
        }

        private delegate void SetExitCodeDelegate(int? exitCode);
        private void setExitCode(int? exitCode)
        {
            if (InvokeRequired)
            {
                Invoke(new SetExitCodeDelegate(setExitCode), exitCode);
                return;
            }

            lbExitCode.Text = string.Format("Process exit code: {0}",
                !exitCode.HasValue ? "undefined" : exitCode.Value.ToString());
            tbLog.Text = string.Empty;
        }

        private void removeItems()
        {
            if (InvokeRequired)
            {
                Invoke(new MethodInvoker(removeItems));
                return;
            }

            lbItems.Items.Clear();
            tbLog.Text = string.Empty;
        }

        public class HistoryItemWrapper
        {
            readonly CoverageReport.RunHistoryMessage item;
            public HistoryItemWrapper(CoverageReport.RunHistoryMessage item)
            {
                this.item = item;
            }

            public string Text
            {
                get { return item.Time.ToLongTimeString() + " " + item.Message; }
            }
        }
    }
}
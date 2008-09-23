using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using PartCover.Browser.Api;
using System.IO;

namespace PartCover.Browser.Features.Views
{
    public partial class RunHistoryView : ReportView
    {
        public RunHistoryView()
        {
            InitializeComponent();
            lbItems.DisplayMember = "Text";
        }

        public override void attach(IServiceContainer container)
        {
            base.attach(container);

            Services.getService<ICoverageReportService>().RunHistory.ItemAdded += HistoryItemAdded;
            Services.getService<ICoverageReportService>().RunHistory.Cleanup += HistoryCleanup;
            Services.getService<ICoverageReportService>().RunHistory.ExitCodeChanged += HistoryExitCodeChanged;
            Services.getService<ICoverageReportService>().RunHistory.DriverLogChanged += HistoryDriverLogChanged;

            setData(Services.getService<ICoverageReportService>().RunHistory);
        }

        public override void detach(IServiceContainer container)
        {
            removeItems();

            Services.getService<ICoverageReportService>().RunHistory.DriverLogChanged -= HistoryDriverLogChanged;
            Services.getService<ICoverageReportService>().RunHistory.ExitCodeChanged -= HistoryExitCodeChanged;
            Services.getService<ICoverageReportService>().RunHistory.Cleanup -= HistoryCleanup;
            Services.getService<ICoverageReportService>().RunHistory.ItemAdded -= HistoryItemAdded;

            base.detach(container);
        }

        delegate void ItemArrayDelegate(IRunHistory history);
        private void setData(IRunHistory history)
        {
            if (InvokeRequired)
            {
                Invoke(new ItemArrayDelegate(setData), history); return;
            }

            foreach (HistoryItem item in history.Items)
                lbItems.Items.Add(new HistoryItemWrapper(item));

            setExitCode(history);
            setLogContent(history.DriverLog);
        }

        private delegate void SString(string value);
        private void setLogContent(string fileName)
        {
            if (InvokeRequired)
            {
                Invoke(new SString(setLogContent), fileName);
                return;
            }

            tbLog.Text = string.Empty;
            tbLog.AppendText("## information from " + fileName);
            tbLog.AppendText(Environment.NewLine);
            tbLog.AppendText(Environment.NewLine);
            tbLog.Text = File.Exists(fileName) ? File.ReadAllText(fileName, Encoding.BigEndianUnicode) : string.Empty;
        }

        private void setExitCode(IRunHistory history)
        {
            if (lbExitCode.InvokeRequired)
            {
                Invoke(new ItemArrayDelegate(setExitCode), history);
                return;
            }

            lbExitCode.Text = string.Format("Process exit code: {0}", !history.ExitCode.HasValue ? "undefined" : history.ExitCode.Value.ToString());
        }

        private void removeItems()
        {
            if (InvokeRequired)
            {
                Invoke(new MethodInvoker(removeItems)); return;
            }
            lbItems.Items.Clear();
        }

        void HistoryExitCodeChanged(object sender, EventArgs e)
        {
            setExitCode((IRunHistory)sender);
        }


        private void HistoryCleanup(object sender, HistoryItemEventArgs e)
        {
            if (InvokeRequired)
            {
                Invoke(new EventHandler<HistoryItemEventArgs>(HistoryCleanup), sender, e);
                return;
            }

            lbItems.Items.Clear();
        }

        private void HistoryItemAdded(object sender, HistoryItemEventArgs e)
        {
            if (InvokeRequired)
            {
                Invoke(new EventHandler<HistoryItemEventArgs>(HistoryItemAdded), sender, e);
                return;
            }

            lbItems.Items.Add(new HistoryItemWrapper(e.Item));
        }

        private void HistoryDriverLogChanged(object sender, EventArgs e)
        {
            setLogContent(((IRunHistory)sender).DriverLog);
        }

        public class HistoryItemWrapper
        {
            readonly HistoryItem item;
            public HistoryItemWrapper(HistoryItem item)
            {
                this.item = item;
            }

            public string Text
            {
                get { return item.Created.ToLongTimeString() + " " + item.Message; }
            }
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            if (sfd.ShowDialog(this) != DialogResult.OK)
                return;
            File.WriteAllText(sfd.FileName, GetPaneText());
        }

        private void btnLoad_Click(object sender, EventArgs e)
        {
            if (ofd.ShowDialog(this) != DialogResult.OK)
                return;
            SetPaneText(File.ReadAllText(ofd.FileName));
        }

        private void SetPaneText(string text)
        {
            if (tcPanes.SelectedTab == tpLog)
            {
                tbLog.Text = text;
                return;
            }

            if (tcPanes.SelectedTab == tpMessages)
            {
                StringReader read = new StringReader(text);
                while (null != (text = read.ReadLine()))
                {
                    HistoryItem item = new HistoryItem();
                    item.Created = DateTime.Now;
                    item.Message = text;
                    lbItems.Items.Add(new HistoryItemWrapper(item));
                }
                return;
            }
        }

        private string GetPaneText()
        {
            if (tcPanes.SelectedTab == tpLog)
            {
                return tbLog.Text;
            }

            if (tcPanes.SelectedTab == tpMessages)
            {
                StringBuilder builder = new StringBuilder();
                foreach (HistoryItemWrapper item in lbItems.Items)
                {
                    builder.AppendLine(item.Text);
                }
                return builder.ToString();
            }

            return string.Empty;
        }
    }
}
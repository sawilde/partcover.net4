using System;
using System.Collections.Generic;
using System.Windows.Forms;
using PartCover.Browser.Api;
using PartCover.Browser.Helpers;
using PartCover.Framework.Data;

namespace PartCover.Browser.Forms
{
    public partial class SkippedItemsReport : Form
    {
        public SkippedItemsReport()
        {
            InitializeComponent();
        }

        public List<SkippedEntry> Items { get; set; }
        public Action<string> RuleReceiver { get; set; }

        private void lbItems_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnAddRule.Enabled = lbItems.SelectedItem != null;
        }

        private void SkippedItemsReport_Shown(object sender, EventArgs e)
        {
            new TinyAsyncUserProcess
            {
                Action = BindData
            }.Execute(this);
        }

        private void BindData(IProgressTracker obj)
        {
            if (Items == null) return;

            Items.ForEach(BindDataItem);
        }

        delegate void BindDataItemDelegate(SkippedEntry obj);
        private void BindDataItem(SkippedEntry obj)
        {
            if (InvokeRequired)
            {
                Invoke(new BindDataItemDelegate(BindDataItem), obj);
                return;
            }

            lbItems.Items.Add(new SkippedEntryWrapper
            {
                Entry = obj
            });
        }

        class SkippedEntryWrapper
        {
            public SkippedEntry Entry { get; set; }

            public override string ToString()
            {
                return string.Format("[{0}]{1}",
                    Entry.AssemblyName,
                    string.IsNullOrEmpty(Entry.TypedefName) ? "*" : Entry.TypedefName);
            }
        }

        private void btnAddRule_Click(object sender, EventArgs e)
        {
            if (RuleReceiver == null)
                return;

            var wrapper = (SkippedEntryWrapper)lbItems.SelectedItem;
            RuleReceiver(wrapper.ToString());
        }
    }
}

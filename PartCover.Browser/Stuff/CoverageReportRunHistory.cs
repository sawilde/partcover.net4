using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Browser.Api;

namespace PartCover.Browser.Stuff
{
    class CoverageReportRunHistory : IRunHistory
    {
        private readonly List<HistoryItem> items = new List<HistoryItem>();
        private int? exitCode;
        private string driverLogFile;

        public string DriverLog
        {
            get { return driverLogFile ?? string.Empty; }
            set { 
                driverLogFile = value;
                if (DriverLogChanged != null) DriverLogChanged(this, EventArgs.Empty);
            }
        }

        public void addMessage(string message)
        {
            HistoryItem item = new HistoryItem();
            item.Created = DateTime.Now;
            item.Message = message;
            items.Add(item);
            if (ItemAdded != null) ItemAdded(this, new HistoryItemEventArgs(item));
        }

        public int? ExitCode
        {
            get { return exitCode; }
            set
            {
                exitCode = value;
                if (ExitCodeChanged != null) ExitCodeChanged(this, EventArgs.Empty);
            }
        }

        public HistoryItem[] Items
        {
            get { return items.ToArray(); }
        }

        public void clear()
        {
            items.Clear();
            if (Cleanup != null) Cleanup(this, new HistoryItemEventArgs());
        }

        public event EventHandler<HistoryItemEventArgs> ItemAdded;

        public event EventHandler<HistoryItemEventArgs> Cleanup;

        public event EventHandler<EventArgs> ExitCodeChanged;

        public event EventHandler<EventArgs> DriverLogChanged;
    }
}

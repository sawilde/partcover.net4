using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public struct HistoryItem
    {
        private string message;
        private DateTime created;

        public string Message
        {
            get { return message; }
            set { message = value; }
        }

        public DateTime Created
        {
            get { return created; }
            set { created = value; }
        }
    }

    public class HistoryItemEventArgs : EventArgs
    {
        private readonly HistoryItem item;
        public HistoryItem Item
        {
            get { return item; }
        }

        public HistoryItemEventArgs(HistoryItem item) { this.item = item; }
        public HistoryItemEventArgs() { }
    }

    public interface IRunHistory
    {
        event EventHandler<HistoryItemEventArgs> ItemAdded;
        event EventHandler<HistoryItemEventArgs> Cleanup;
        event EventHandler<EventArgs> ExitCodeChanged;
        event EventHandler<EventArgs> DriverLogChanged;

        void addMessage(string message);

        void clear();

        HistoryItem[] Items { get;}

        int? ExitCode { get;set;}

        string DriverLog { get; set;}
    }
}

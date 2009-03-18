using System;
using PartCover.Framework.Walkers;

namespace PartCover.Framework
{
    public class EventArgs<T> : EventArgs
    {
        private readonly T data;

        public T Data { get { return data; } }

        public EventArgs(T data) { this.data = data; }
    }

    public class ProcessCallback
    {
        public event EventHandler<EventArgs<CoverageReport.RunHistoryMessage>> OnMessage;

        internal void writeStatus(string message)
        {
            if (OnMessage == null)
                return;

            var data = new CoverageReport.RunHistoryMessage 
            {
                Message = message, 
                Time = DateTime.Now.ToUniversalTime()
            };
            OnMessage(this, new EventArgs<CoverageReport.RunHistoryMessage>(data));
        }
    }
}

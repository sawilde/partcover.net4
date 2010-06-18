using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public class StatusEventArgs : EventArgs<string>
    {
        public StatusEventArgs(string data) : base(data) { }
    }

    public class LogEntryEventArgs : EventArgs<LogEntry>
    {
        public LogEntryEventArgs(LogEntry data) : base(data) { }
    }

    public partial class Connector
    {
        public void SetLogging(Logging logging)
        {
            connector.LoggingLevel = (int)logging;
        }

        public void UseFileLogging(bool logging)
        {
            connector.FileLoggingEnable = logging;
        }

        public void UsePipeLogging(bool logging)
        {
            connector.PipeLoggingEnable = logging;
        }

        public void IncludeItem(string item)
        {
            connector.IncludeItem(item);
        }

        public void ExcludeItem(string item)
        {
            connector.ExcludeItem(item);
        }
    }
}
namespace PartCover.Framework.Data
{
    public class LogEntry
    {
        public string Message { get; set; }
        public int MsOffset { get; set; }
        public int ThreadId { get; set; }

        public string ToHumanString()
        {
            return string.Format("[{0:00000}] [{1:00000}] {2}", MsOffset, ThreadId, Message);
        }
    }
}
namespace PartCover.Browser.Api
{
    public interface IProgressTracker
    {
        void AppendMessage(string message);

        void QueueBegin(string message);
        void QueuePush(string message);
        void QueueEnd(string message);

        float Percent { get; set; }
    }

    public class DummyProgressTracker : IProgressTracker
    {
        public void AppendMessage(string message)
        {
        }

        public void QueueBegin(string message)
        {
        }

        public void QueuePush(string message)
        {
        }

        public void QueueEnd(string message)
        {
        }

        public float Percent
        {
            get { return 0; }
            set { }
        }
    }
}

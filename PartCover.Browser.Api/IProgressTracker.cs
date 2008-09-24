using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public interface IProgressTracker
    {
        void setMessage(string message);

        void queueBegin(string message);
        void queuePush(string message);
        void queueEnd(string message);

        void setPercent(float value);
        float getPercent();
    }

    public class DummyProgressTracker : IProgressTracker
    {
        public void setMessage(string message) { }

        public void setPercent(float value) { }

        public float getPercent() { return 0; }

        public void queueBegin(string message) { }

        public void queuePush(string message) { }

        public void queueEnd(string message) { }
    }
}

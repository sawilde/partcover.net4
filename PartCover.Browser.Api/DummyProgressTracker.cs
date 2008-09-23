using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public class DummyProgressTracker : IProgressTracker
    {
        public void setMessage() { }

        public void setPercent(float value) { }

        public float getPercent() { return 0; }
    }
}

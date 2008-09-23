using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public interface IProgressTracker
    {
        void setMessage();
        void setPercent(float value);
        float getPercent();
    }
}

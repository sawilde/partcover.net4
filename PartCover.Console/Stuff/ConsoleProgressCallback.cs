using System;
using PartCover.Framework;

namespace PartCover.Stuff
{
    class ConsoleProgressCallback : IProgressCallback {
        public void writeStatus(string value)
        {
            Console.Out.WriteLine(value);
        }
    }
}

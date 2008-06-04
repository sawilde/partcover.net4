using System;
using System.IO;

namespace PartCover.Framework
{
	public class Connector
	{
        PartCoverConnectorClass connector = new PartCover.PartCoverConnectorClass();

        public Connector() {}

        private System.IO.TextWriter outWriter;
        public System.IO.TextWriter Out {
            get { return outWriter; }
            set { outWriter = value; }
        }

        public void SetVerbose(int level) {
            connector.SetVerbose(level);
        }

        private Walkers.InstrumentedBlocksWalkerInner blockWalker = new Walkers.InstrumentedBlocksWalkerInner();
        public Walkers.InstrumentedBlocksWalker BlockWalker {
            get { return blockWalker; }
        }

        public void StartTarget(string path, string directory, string args, bool redirectOutput, bool delayClose) {
            // set mode
            connector.EnableOption(ProfilerMode.COUNT_COVERAGE);
            //connector.EnableOption(ProfilerMode.COUNT_CALL_DIAGRAM);

            ExcludeItem("[mscorlib]*");
            ExcludeItem("[System*]*");

            if (directory != null) directory = directory.Trim();
            if (path != null) path = path.Trim();
            if (args != null) args = args.Trim();

            if (directory == null || directory.Length == 0)
                directory = Directory.GetCurrentDirectory();

            // start target
            if (Out != null) Out.WriteLine("{0}: Start target", DateTime.Now);
            connector.StartTarget(path, directory, args, redirectOutput);

            // wait results
            if (Out != null) Out.WriteLine("{0}: Receive results", DateTime.Now);
            connector.WaitForResults(delayClose);

            // walk results
            if (Out != null) Out.WriteLine("{0}: Walk results", DateTime.Now);
            connector.WalkInstrumentedResults(blockWalker);
        }

        public void CloseTarget() {
            connector.CloseTarget();
        }

        public void IncludeItem(string item) {
            connector.IncludeItem(item);
        }

        public void ExcludeItem(string item) {
            connector.ExcludeItem(item);
        }
    }
}

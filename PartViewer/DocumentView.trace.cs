using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace PartViewer
{
    partial class DocumentView 
    {
        internal class ViewControlTrace
        {
            private readonly DocumentView host;

            public ViewControlTrace(DocumentView host)
            {
                this.host = host;
                enabled = true;
            }

            int viewCreated;

            readonly Stopwatch stopwatch = new Stopwatch();
            private bool enabled;

            public bool Enabled
            {
                get { return enabled; }
                set { enabled = value; }
            }

            [Conditional("DEBUG")]
            public void beginPaint(Rectangle rectangle)
            {
                if (!enabled) 
                    return;

                Trace.WriteLine("PV Updating Clip: " + rectangle);
                Trace.WriteLine("            Cursor: " + host.Position);
                Trace.WriteLine("            Top Offset: " + host.renderStuff.FirstRow + " rows");
                Trace.WriteLine("            Left Offset: " + host.renderStuff.LeftOffset + " pixels");

                viewCreated = 0;
                stopwatch.Reset();
                stopwatch.Start();
            }

            [Conditional("DEBUG")]
            public void endPaint()
            {
                if (!enabled) 
                    return;

                stopwatch.Stop();
                Trace.WriteLine("PV Updated Time: " + stopwatch.ElapsedMilliseconds);
                Trace.WriteLine("           VC: " + viewCreated);
                Trace.WriteLine("           SD: " + 0);
                Trace.WriteLine("           LRO: " + host.renderStuff.LastRow);
            }

            [Conditional("DEBUG")]
            public void createDocumentRowViewRowView()
            {
                if (!enabled) return;

                viewCreated++;
            }

            [Conditional("DEBUG")]
            public void keyDown(int charCode, Keys e)
            {
                if (!enabled) return;

                Trace.WriteLine("PV KeyDown " + charCode + " [" + e + "]");
            }

            [Conditional("DEBUG")]
            public void keyPress(KeyPressEventArgs e)
            {
                if (!enabled) return;

                Trace.WriteLine("PV KeyPress " + e);
            }

            [Conditional("DEBUG")]
            public void keyUp(int charCode, Keys e)
            {
                if (!enabled) return;

                Trace.WriteLine("PV KeyUp " + charCode + " [" + e + "]");
            }

            [Conditional("DEBUG")]
            public void cursorMoving(int dx, int dy)
            {
                if (!enabled) return;

                Trace.WriteLine("PV Cursor Moving by " + new Point(dx, dy));
            }

            [Conditional("DEBUG")]
            public void cursorMoved()
            {
                if (!enabled) return;

                Trace.WriteLine("PV Cursor Moved to " + host.Position);
            }

            [Conditional("DEBUG")]
            public void write(string s)
            {
                if (!enabled) return;

                Trace.WriteLine(s);
            }
        }
    }
}

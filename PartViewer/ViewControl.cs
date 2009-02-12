using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using PartViewer.Model;

namespace PartViewer
{
    public partial class ViewControl : UserControl
    {
        private class ScrollHostProxy : IScrollable
        {
            readonly ViewControl owner;
            public ScrollHostProxy(ViewControl owner)
            {
                this.owner = owner;
            }

            public void setScroll(Size bounds, int h, int v)
            {
                owner.setScroll(bounds, h, v);
            }
        }

        public ViewControl()
        {
            InitializeComponent();


            SetStyle(ControlStyles.ContainerControl, false);

            hScrollBar.Left = 0;
            hScrollBar.Top = Height - SystemInformation.HorizontalScrollBarHeight;
            hScrollBar.Height = SystemInformation.HorizontalScrollBarHeight;
            hScrollBar.Width = Width - SystemInformation.VerticalScrollBarWidth;

            vScrollBar.Left = Width - SystemInformation.VerticalScrollBarWidth;
            vScrollBar.Top = 0;
            vScrollBar.Width = SystemInformation.VerticalScrollBarWidth;
            vScrollBar.Height = Height - SystemInformation.HorizontalScrollBarHeight;

            text.Location = new Point(0, 0);
            text.Size = new Size(hScrollBar.Width, vScrollBar.Height);
            text.ScrollHost = new ScrollHostProxy(this);

            text.CaretMoved += text_CaretMoved;

        }

        public event EventHandler<EventArgs> CaretMoved;

        void text_CaretMoved(object sender, EventArgs e)
        {
            if (null != CaretMoved) CaretMoved(this, e);
        }

        [Browsable(false)]
        public Document Document
        {
            [DebuggerHidden]
            get { return text.Document; }
            set { text.Document = value; }
        }

        [Browsable(false)]
        public Model.IView View
        {
            [DebuggerHidden]
            get { return text.View; }
        }

        [DefaultValue(false)]
        public bool TraceEnabled
        {
            [DebuggerHidden]
            get { return text.TraceEnabled; }
            [DebuggerHidden]
            set { text.TraceEnabled = value; }
        }

        private void setScroll(Size bounds, int h, int v)
        {
            vScrollBar.Maximum = Math.Max(bounds.Height, 0);
            hScrollBar.Maximum = Math.Max(bounds.Width, 0);

            v = Math.Min(v, vScrollBar.Maximum);
            h = Math.Min(h, hScrollBar.Maximum);

            vScrollBar.Value = v;
            hScrollBar.Value = h;
        }

        private void setViewScrollValues(object sender, ScrollEventArgs e)
        {
            Size newValues = Size.Empty;
            if (sender == hScrollBar)
            {
                newValues.Width = e.NewValue;
                newValues.Height = vScrollBar.Value;
            }
            else
            {
                newValues.Height = e.NewValue;
                newValues.Width = hScrollBar.Value;
            }
            text.Scroll(newValues.Width, newValues.Height);
        }
    }
}

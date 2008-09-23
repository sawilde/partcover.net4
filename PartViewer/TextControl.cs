using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using PartViewer.Styles;

namespace PartViewer
{
    internal class TextControl : Control
    {
        private DocumentView view;
        private ViewControl.ScrollHost scrollHost;
        private Graphics graphics;
        
        public TextControl()
        {
            view = new DocumentView(new DocumentViewSurface(this));

            SetStyle(
                ControlStyles.AllPaintingInWmPaint |
                ControlStyles.UserPaint |
                ControlStyles.OptimizedDoubleBuffer |
                ControlStyles.ContainerControl |
                ControlStyles.ResizeRedraw 
                , true);
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                if (null != graphics)
                {
                    graphics.Dispose();
                    graphics = null;
                }
                view = null;
            }
        }

        internal ViewControl.ScrollHost ScrollHost
        {
            get { return scrollHost; }
            set { scrollHost = value; }
        }

        public ViewStyle ViewStyle
        {
            get { return view.ViewStyle; }
        }

        [Browsable(false)]
        public Document Document
        {
            [DebuggerHidden]
            get { return view.Document; }
            set { view.Document = value; }
        }

        [Browsable(false)]
        public Model.View View
        {
            get { return view; }
        }

        [DefaultValue(false)]
        public bool TraceEnabled
        {
            get { return view.TraceEnabled; }
            set { view.TraceEnabled = value; }
        }

        private Rectangle ViewRectangle
        {
            get {
                Rectangle cs = ClientRectangle;
                cs.Height = Math.Max(cs.Height, 0);
                cs.Width = Math.Max(cs.Width, 0);
                return cs;
            }
        }

        protected override void OnPaintBackground(PaintEventArgs e) { }

        protected override void OnPaint(PaintEventArgs e)
        {
            Rectangle clip = e.ClipRectangle;
            clip.Intersect(ViewRectangle);
            view.draw(e.Graphics, clip);
        }

        protected override void OnClientSizeChanged(EventArgs e)
        {
            if (view != null)
                view.Bounds = ViewRectangle.Size;
            base.OnClientSizeChanged(e);
        }

        protected override void OnGotFocus(EventArgs e)
        {
            invalidate();
            base.OnGotFocus(e);
        }

        protected override void OnLostFocus(EventArgs e)
        {
            invalidate();
            base.OnLostFocus(e);
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            Focus();
            moveCursorUnderMouse(e.Location);
            base.OnMouseDown(e);
        }

        private void moveCursorUnderMouse(Point pt)
        {
            pt.Offset(ViewRectangle.Location);
            pt = view.getNearestCaretPosition(pt);
            view.moveCaretTo(pt);
        }

        #region Handle Keys
        protected override bool IsInputChar(char charCode)
        {
            return true;
        }

        protected override bool IsInputKey(Keys keyData)
        {
            return true;
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            view.keyDown(e.KeyValue, e.Modifiers);
            base.OnKeyDown(e);
        }

        protected override void OnKeyUp(KeyEventArgs e)
        {
            view.keyUp(e.KeyValue, e.Modifiers);
            base.OnKeyUp(e);
        }
        #endregion Handle Keys

        internal void invalidate()
        {
            Invalidate();
        }

        internal Graphics Graphics
        {
            [DebuggerHidden]
            get { return graphics ?? (graphics = CreateGraphics()); }
        }

        internal event EventHandler<EventArgs> CaretMoved
        {
            add { view.CaretMoved += value; }
            remove { view.CaretMoved -= value; }
        }

        internal void setScroll(Size bounds, int h, int v)
        {
            if (scrollHost == null) return;
            scrollHost.setScroll(bounds, h, v);
        }

        internal void scroll(int hValue, int vValue)
        {
            view.scroll(hValue, vValue);
        }
    }
}
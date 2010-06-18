using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;

using PartViewer.Model;
using PartViewer.Plugins;

namespace PartViewer
{
    public partial class DocumentView : IView
    {
        //(StringFormat)StringFormat.GenericTypographic.Clone();
        private static readonly StringFormat stringFormat = new StringFormat
        {
            Trimming = StringTrimming.None,
            LineAlignment = StringAlignment.Near,
            Alignment = StringAlignment.Near,
            FormatFlags = StringFormatFlags.MeasureTrailingSpaces | StringFormatFlags.NoFontFallback | StringFormatFlags.FitBlackBox | StringFormatFlags.LineLimit | StringFormatFlags.NoClip
        };

        private readonly ViewControlTrace trace;
        private readonly Dictionary<int, DocumentRowView> renderList;
        private readonly DocumentViewSurface surface;

        private Size bounds;
        private RenderStuff renderStuff;

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private Document document;

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private readonly Style selectionStyle;

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private readonly ViewStyle viewStyle;

        #region External

        readonly IViewPlugin[] plugins = new[] { 
            new PcSelectionMode()
        };

        #endregion External

        internal DocumentView(DocumentViewSurface surface)
        {
            this.surface = surface;

            renderStuff = new RenderStuff(this);
            trace = new ViewControlTrace(this);
            keyActionMap = new KeyActionMap();
            renderList = new Dictionary<int, DocumentRowView>();

            createKeyCommands();

            ClearStuff();

            selectionStyle = new Style
            {
                Foreground = Color.White,
                Background = Color.Blue
            };

            viewStyle = new ViewStyle();
        }

        public Size Bounds
        {
            [DebuggerHidden]
            get { return bounds; }
            set
            {
                bounds = value;
                surface.invalidate();
            }
        }

        public Document Document
        {
            [DebuggerHidden]
            get { return document; }

            set
            {
                if (document == value) return;

                UnadviseDocumentEvents();
                DetachViewCommands();
                ClearStuff();
                document = value;
                AdviseDocumentEvents();
                AttachViewCommands();
                surface.invalidate();
            }
        }

        public ViewStyle ViewStyle
        {
            [DebuggerHidden]
            get { return viewStyle; }
        }

        public Style SelectionStyle
        {
            [DebuggerHidden]
            get { return selectionStyle; }
        }

        private DocumentRow CurrentRow
        {
            [DebuggerHidden]
            get { return document == null ? null : document.Rows[Position.Y]; }
        }

        public bool TraceEnabled
        {
            get { return trace.Enabled; }
            set { trace.Enabled = value; }
        }

        private void ClearStuff()
        {
            renderList.Clear();
            renderStuff.clear();
            Position = new Point(0, 0);
            selection = new SelectionRegion();
        }

        private void AdviseDocumentEvents()
        {
            if (document == null) return;

            document.FaceChanged += document_FaceChanged;
        }

        private void UnadviseDocumentEvents()
        {
            if (document == null) return;

            document.FaceChanged -= document_FaceChanged;
        }

        private void AttachViewCommands()
        {
            if (document == null) return;

            attachKeyCommands();
            foreach (var p in plugins)
                p.Attach(this);
        }

        private void DetachViewCommands()
        {
            if (document == null) return;

            foreach (var p in plugins)
                p.Detach(this);
            detachKeyCommands();
        }

        public Point Position { get; private set; }

        public void MoveCaretTo(Point pt)
        {
            moveCursorVertical(pt.Y - Position.Y, false);
            moveCursorHorizont(pt.X - Position.X);
            surface.invalidate();
        }

        public void CenterLine(int line)
        {
            if (Document == null) return;

            var heightLimit = bounds.Height / 2.0f;
            do
            {
                var rowView = getDocumentRowView(line);
                if (rowView == null) break;

                heightLimit -= rowView.bounds.Height;
                line--;
            }
            while (heightLimit > 0 && line > 0);

            renderStuff.FirstRow = Math.Max(0, line);
            surface.invalidate();
        }


        private DocumentRowView getDocumentRowView(int offset)
        {
            DocumentRowView rowView;
            renderList.TryGetValue(offset, out rowView);
            return rowView;
        }

        public void Scroll(int hValue, int vValue)
        {
            renderStuff.setScroll(hValue, vValue);
            surface.invalidate();
        }
    }
}
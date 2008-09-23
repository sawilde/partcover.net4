using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;

using PartViewer.Model;
using PartViewer.Plugins;
using PartViewer.Styles;

namespace PartViewer
{
    public partial class DocumentView : View
    {
        private static readonly StringFormat stringFormat;

        [DebuggerHidden]
        static DocumentView()
        {
            stringFormat = (StringFormat)StringFormat.GenericTypographic.Clone();
            stringFormat.Trimming = StringTrimming.None;
            stringFormat.FormatFlags |= StringFormatFlags.MeasureTrailingSpaces;
            stringFormat.FormatFlags |= StringFormatFlags.NoFontFallback;
        }

        private Point caret;
        private bool caretVisible;

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

        readonly ViewPlugin[] plugins = new ViewPlugin[] { 
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

            selectionStyle = new Style();
            selectionStyle.Foreground = Color.White;
            selectionStyle.Background = Color.Blue;

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
            get { return document == null ? null : document.Rows[caret.Y]; }
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
            caret = new Point(0, 0);
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
            foreach (ViewPlugin p in plugins)
                p.attach(this);
        }

        private void DetachViewCommands()
        {
            if (document == null) return;

            foreach (ViewPlugin p in plugins)
                p.detach(this);
            detachKeyCommands();
        }

        public Point Caret
        {
            [DebuggerHidden]
            get
            {
                return caret;
            }
        }

        public void moveCaretTo(Point pt)
        {
            moveCursorVertical(pt.Y - Caret.Y, false);
            moveCursorHorizont(pt.X - Caret.X);
            surface.invalidate();
        }

        public void centerLine(int line)
        {
            if (Document == null) return;

            float heightLimit = bounds.Height / 2;
            do
            {
                DocumentRowView rowView = getDocumentRowView(line);
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

        public void scroll(int hValue, int vValue)
        {
            renderStuff.setScroll(hValue, vValue);
            surface.invalidate();
        }
    }
}
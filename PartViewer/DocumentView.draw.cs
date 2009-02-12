using System;
using System.Collections.Generic;
using System.Drawing;
using PartViewer.Model;
using PartViewer.Utils;

namespace PartViewer
{
    partial class DocumentView
    {
        #region DocumentRowView

        internal class DocumentRowView
        {
            public readonly DocumentRow row;

            public StylizedRowElement[] parts;
            public string[] partRaws;
            public SizeF[] partSizes;

            public RectangleF bounds;

            public bool dirty = true;

            public DocumentRowView(DocumentRow row)
            {
                this.row = row;
            }
        }
        #endregion DocumentRowView

        #region Render Stuff
        private struct RenderStuff
        {
            const float HorizontalKoeff = 20.0f;

            private int firstRow;
            private int lastRow;

            private float leftOffset;
            private float lineFrame;
            private bool scrolling;

            public RectangleF caretRect;

            private readonly DocumentView host;

            public RenderStuff(DocumentView host)
                : this()
            {
                this.host = host;
            }

            public void clear()
            {
                firstRow = 0;
                lastRow = 0;
                leftOffset = 0;
                lineFrame = 0;
                caretRect = RectangleF.Empty;
                host.surface.setScroll(Size.Empty, 0, 0);
            }

            public float LeftOffset
            {
                get { return leftOffset; }
                set { leftOffset = Math.Max(value, 0); }
            }

            public float LineFrame
            {
                private get { return lineFrame; }
                set { lineFrame = Math.Max(value, lineFrame); }
            }

            public int LastRow
            {
                get { return lastRow; }
                set { lastRow = Math.Max(value, 0); }
            }

            public int FirstRow
            {
                get { return firstRow; }
                set
                {
                    firstRow = Math.Max(value, 0);
                    updateScroll();
                }
            }

            public void updateScroll()
            {
                if (scrolling) return;

                var range = Size.Empty;

                range.Width = (int)Math.Ceiling(LineFrame);
                range.Height = host.Document.LineCount;

                var k = (int)Math.Floor(host.bounds.Width / HorizontalKoeff);
                var h = (int)Math.Floor(LeftOffset);

                if (host.bounds.Width > 0)
                {
                    h /= k;
                    range.Width /= k;
                }

                host.surface.setScroll(range, h, FirstRow);
            }

            public void setScroll(int hValue, int vValue)
            {
                scrolling = true;

                var k = (int)Math.Floor(host.bounds.Width / HorizontalKoeff);

                try
                {
                    FirstRow = vValue;
                    LeftOffset = hValue * k;
                }
                finally
                {
                    scrolling = false;
                }
            }
        }
        #endregion Render Stuff

        public void Draw(Graphics gr, Rectangle clip)
        {
            if (document == null)
            {
                gr.Clear(Color.White);
                return;
            }

            gr.Clear(document.Style.Background);

            trace.beginPaint(clip);

            float thisPixelOffset = 0;

            renderStuff.LastRow = renderStuff.FirstRow;

            while (thisPixelOffset < Bounds.Height)
            {
                var rowView = getDocumentRowView(renderStuff.LastRow);
                if (rowView == null)
                {
                    renderList[renderStuff.LastRow] = rowView =
                        createRowView(renderStuff.LastRow);
                }

                if (rowView == null)
                {
                    break;
                }

                if (rowView.dirty)
                {
                    prepare(gr, rowView);
                }

                CharacterRange selectionRange;
                if (ViewStyle.HideInactiveSelection && !surface.hasFocus())
                {
                    selectionRange = new CharacterRange();
                }else 
                {
                    selectionRange = Selection.ExtractLineRange(rowView.row.Index, rowView.row.Length);
                }

                var hideCaret = ViewStyle.HideInactiveCursor && !surface.hasFocus();

                if (!hideCaret && rowView.row.Index == Position.Y)
                {
                    RenderRowViewWithCursor(gr, rowView, -renderStuff.LeftOffset, Position.X, selectionRange);
                }
                else
                {
                    RenderRowView(gr, rowView, -renderStuff.LeftOffset, selectionRange);
                }

                gr.TranslateTransform(0, rowView.bounds.Height);

                thisPixelOffset += rowView.bounds.Height;

                if (thisPixelOffset < bounds.Height)
                {
                    renderStuff.LastRow++;
                }
            }

            renderStuff.LastRow = Math.Max(0, renderStuff.LastRow - 1);
            renderStuff.updateScroll();

            trace.endPaint();
        }

        private SizeF measure(Graphics g, DocumentRow row, StylizedRowElement atom)
        {
            return MeasureStringPart(new CharacterRange(0, atom.range.Length),
                g, row.Raw, atom.style).Size;
        }

        private void RenderRowView(
            Graphics g,
            DocumentRowView view,
            float offset,
            CharacterRange selectionRange)
        {
            for (var i = 0; i < view.parts.Length; ++i)
            {
                var atom = view.parts[i];
                var atomRaw = view.partRaws[i];

                var trio = intersectAtomWithSelection(atom.range, selectionRange);
                if (trio.first.Length > 0)
                {
                    trio.first.First -= atom.range.First;

                    offset += DrawStringPart(trio.first, g, atomRaw, atom.style, offset).Width;
                }

                if (trio.second.Length > 0)
                {
                    trio.second.First -= atom.range.First;

                    Style thisStyle = atom.style.Combine(selectionStyle);

                    offset += DrawStringPart(trio.second, g, atomRaw, thisStyle, offset).Width;
                }

                if (trio.third.Length > 0)
                {
                    trio.third.First -= atom.range.First;

                    offset += DrawStringPart(trio.third, g, atomRaw, atom.style, offset).Width;
                }
            }
            renderStuff.LineFrame = offset;
        }

        private void RenderRowViewWithCursor(
            Graphics g,
            DocumentRowView view,
            float offset,
            int cursor,
            CharacterRange selectionRange)
        {
            for (int i = 0, last = view.parts.Length - 1; i <= last; ++i)
            {
                var atom = view.parts[i];
                var atomRaw = view.partRaws[i];

                if (i == last && cursor >= atom.range.Length)
                {
                    cursor = atom.range.Length - 1;
                }

                var trio = intersectAtomWithSelection(atom.range, selectionRange);

                if (trio.first.Length > 0) {
                    trio.first.First -= atom.range.First;

                    var thisStyle = atom.style;

                    var offsetChange = DrawStringPart(trio.first, g, atomRaw, thisStyle, offset).Width;

                    cursor -= drawCursor(cursor, trio.first, offset, g, atomRaw, thisStyle);

                    offset += offsetChange;
                }

                if (trio.second.Length > 0)
                {
                    trio.second.First -= atom.range.First;

                    var thisStyle = atom.style.Combine(selectionStyle);

                    var offsetChange = DrawStringPart(trio.second, g, atomRaw, thisStyle, offset).Width;

                    cursor -= drawCursor(cursor, trio.second, offset, g, atomRaw, thisStyle);

                    offset += offsetChange;
                }

                if (trio.third.Length > 0)
                {
                    trio.third.First -= atom.range.First;

                    var thisStyle = atom.style;

                    var offsetChange = DrawStringPart(trio.third, g, atomRaw, atom.style, offset).Width;

                    cursor -= drawCursor(cursor, trio.third, offset, g, atomRaw, thisStyle);

                    offset += offsetChange;
                }
            }
            renderStuff.LineFrame = offset;
        }

        private int drawCursor(int cursor, CharacterRange part, float offset, Graphics g, string atomRaw, Style thisStyle)
        {
            if (cursor < 0 || cursor >= part.Length)
                return part.Length;

            renderStuff.caretRect = MeasureStringPart(new CharacterRange(cursor, 1), g, atomRaw, thisStyle);

            var caretRect = renderStuff.caretRect;
            caretRect.Offset(offset, 0);

            var pen = PenCache.GetSolid(Color.Black);
            pen.Width = 2;

            g.DrawLine(pen, caretRect.Left, caretRect.Top, caretRect.Left, caretRect.Bottom);

            return part.Length;
        }

        private static CharacterRangeTrio intersectAtomWithSelection(CharacterRange range, CharacterRange selectionRange)
        {
            var trio = new CharacterRangeTrio();

            if (selectionRange.Length == 0 
                || range.First >= selectionRange.First + selectionRange.Length
                || range.First + range.Length <= selectionRange.First)
            {
                trio.first = range;
                return trio;
            }

            if (range.First < selectionRange.First)
            {
                trio.first = new CharacterRange(range.First, selectionRange.First - range.First);
            }

            trio.second = intersect(range, selectionRange);

            if (trio.second.Length > 0 && trio.second.First + trio.second.Length < range.First + range.Length)
            {
                trio.third = new CharacterRange(trio.second.First + trio.second.Length,
                    (range.First + range.Length) - (trio.second.First + trio.second.Length));
            }

            return trio;
        }

        private static CharacterRange intersect(CharacterRange range1, CharacterRange range2)
        {
            var start = Math.Max(range1.First, range2.First);
            var end = Math.Min(range1.First + range1.Length, range2.First + range2.Length);

            return end > start 
                ? new CharacterRange(start, end - start) 
                : new CharacterRange();
        }

        private RectangleF DrawStringPart(CharacterRange range, Graphics g, string str, Style style, float offset)
        {
            string part = str.Substring(range.First, range.Length);

            UpdateTabStop(part, style);

            RectangleF size = MeasureStringPart(range, g, str, style);

            if (!style.Background.IsEmpty)
            {
                RectangleF textRect = size;
                textRect.X = 0;
                textRect.Offset(offset, 0);
                g.FillRectangle(BrushCache.getSolid(style.Background), textRect);
            }

            g.DrawString(part, FontCache.get(style), BrushCache.getForeground(style), offset, 0, stringFormat);

            return size;
        }

        private void UpdateTabStop(string str, Style style)
        {
            var tabCount = countTabs(str);
            if (tabCount <= 0)
            {
                return;
            }

            var tabString = new string(' ', ViewStyle.TabSize);
            var tabSize = surface.Graphics.MeasureString(tabString, FontCache.get(style), 0, stringFormat);
            stringFormat.SetTabStops(0, new[] { tabSize.Width });
        }

        private static int countTabs(string part)
        {
            var tabCount = 0;
            var indexOfTab = -1;
            while (-1 != (indexOfTab = part.IndexOf('\t', indexOfTab + 1)))
            {
                tabCount++;
            }
            return tabCount;
        }

        private RectangleF[] MeasureStringParts(CharacterRange[] range, Graphics g, string str, Style style)
        {
            UpdateTabStop(str, style);

            stringFormat.SetMeasurableCharacterRanges(range);

            Region[] regn = g.MeasureCharacterRanges(str, FontCache.get(style),
                                                     RectangleF.Empty, stringFormat);

            return Array.ConvertAll(regn, rgn => rgn.GetBounds(g));
        }

        private RectangleF MeasureStringPart(CharacterRange range, string str, Style style)
        {
            return MeasureStringPart(range, surface.Graphics, str, style);
        }

        private RectangleF MeasureStringPart(CharacterRange range, Graphics g, string str, Style style)
        {
            if (range.Length == 0)
                return RectangleF.Empty;
            return MeasureStringParts(new[] { range }, g, str, style)[0];
        }

        private void prepare(Graphics g, DocumentRowView view)
        {
            view.parts = document.getStylizedRow(view.row.Index);

            SizeF sum = SizeF.Empty;

            var partRawList = new List<string>();
            var partSizeList = new List<SizeF>();
            foreach (var atom in view.parts)
            {
                var atomSize = measure(g, view.row, atom);
                sum.Width += atomSize.Width;
                if (sum.Height < atomSize.Height)
                    sum.Height = atomSize.Height;

                partRawList.Add(view.row.Raw.Substring(atom.range.First, atom.range.Length));
                partSizeList.Add(atomSize);
            }

            view.partRaws = partRawList.ToArray();
            view.partSizes = partSizeList.ToArray();
            view.bounds.Size = sum;

            var prevView = getDocumentRowView(view.row.Index - 1);
            if (prevView != null)
                view.bounds.Y = prevView.bounds.Bottom;

            view.dirty = false;
        }

        private DocumentRowView createRowView(int rowIndex)
        {
            var row = document.Rows[rowIndex];
            if (row == null)
                return null;

            trace.createDocumentRowViewRowView();

            return new DocumentRowView(row);
        }

        void document_FaceChanged(object sender, DocumentRow arg)
        {
            var view = getDocumentRowView(arg.Index);
            if (view != null)
                view.dirty = true;
            surface.invalidate();
        }
    }
}

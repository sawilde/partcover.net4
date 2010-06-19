using System;
using System.Collections.Generic;
using System.Drawing;

using PartViewer.Model;

namespace PartViewer
{
    internal class StylizedRowElement
    {
        public CharacterRange range;
        public Style style;

        public StylizedRowElement(CharacterRange range, Style style)
        {
            this.range = range;
            this.style = style;
        }
    }

    partial class Document
    {
        private struct FaceApplied
        {
            private readonly string face;
            private readonly CharacterRange range;

            public string Face { get { return face; } }
            public CharacterRange Range { get { return range; } }

            public FaceApplied(string face, CharacterRange range)
            {
                this.face = face;
                this.range = range;
            }
        }

        private class StylizerSourceImpl : IStylizerSource
        {
            readonly Document doc;
            readonly DocumentRange range;

            public StylizerSourceImpl(Document doc, DocumentRange range)
            {
                this.doc = doc;
                this.range = range;
            }

            public Document Document { get { return doc; } }
            public DocumentRange Range { get { return range; } }

            public void AssignFace(DocumentRange target, string face) { doc.AssignFace(target, face); }
            public void AssignFace(DocumentRange target, IStyleFace face) { doc.AssignFace(target, face); }
        }

        private class Face : IStyleFace
        {
            private readonly string name;
            private readonly IStylizer owner;
            private readonly Style faceStyle;

            public string Name
            {
                get { return name; }
            }

            public IStylizer Owner
            {
                get { return owner; }
            }

            public Style FaceStyle
            {
                get { return faceStyle; }
            }

            public Face(string name, IStylizer owner)
            {
                this.name = name;
                this.owner = owner;
                faceStyle = new Style();
            }
        }

        readonly List<IStylizer> stylizers = new List<IStylizer>();
        readonly Dictionary<int, List<FaceApplied>> stylesApplied = new Dictionary<int, List<FaceApplied>>();
        readonly Dictionary<string, Face> faces = new Dictionary<string, Face>();

        public IStyleFace CreateFace(IStylizer stylizer, string name)
        {
            Face face;
            if (faces.ContainsKey(name))
                return null;

            faces[name] = face = new Face(name, stylizer);
            return face;
        }

        public IStyleFace FindFace(string name)
        {
            return faces[name];
        }

        public void Add(IStylizer stylizer)
        {
            stylizers.Add(stylizer);
            updateStyle(stylizer, 0, 0);
        }

        public void Remove(IStylizer stylizer)
        {
            stylizers.Remove(stylizer);

            var faceToRemove = new List<string>();
            foreach (var f in faces)
            {
                if (f.Value.Owner == stylizer)
                    faceToRemove.Add(f.Key);
            }

            var rowList = new Dictionary<int, bool>();
            foreach (var s in faceToRemove)
            {
                foreach (var sa in stylesApplied)
                {
                    var value = s;
                    if (sa.Value.RemoveAll(fa => fa.Face.Equals(value)) > 0)
                    {
                        rowList[sa.Key] = true;
                    }
                }
                faces.Remove(s);
            }

            foreach (var a in rowList)
            {
                if (stylesApplied.ContainsKey(a.Key) && stylesApplied[a.Key].Count == 0)
                    stylesApplied.Remove(a.Key);
            }

            foreach (var a in rowList)
            {
                fireFaceChanged(Rows[a.Key]);
            }
        }

        public void RemoveStylizerAll()
        {
            foreach (var s in new List<IStylizer>(Stylizers))
                Remove(s);
        }

        public ICollection<IStylizer> Stylizers
        {
            get { return stylizers.AsReadOnly(); }
        }

        internal StylizedRowElement[] getStylizedRow(int line)
        {
            return combine(Rows[line], getStylesApplied(line));
        }

        private List<FaceApplied> getStylesApplied(int line)
        {
            List<FaceApplied> res;
            stylesApplied.TryGetValue(line, out res);
            return res;
        }

        private StylizedRowElement[] combine(DocumentRow row, ICollection<FaceApplied> styles)
        {
            if (styles == null || styles.Count == 0)
            {
                var result = new StylizedRowElement[1];
                result[0] = new StylizedRowElement(new CharacterRange(0, row.Length), style);
                return result;
            }

            var elements = new List<CharacterRange>
            {
                new CharacterRange(0, row.Length)
            };

            foreach (var styleApplied in styles)
            {
                var faceRange = styleApplied.Range;

                for (var i = 0; i < elements.Count; ++i)
                {
                    CharacterRange element = elements[i];

                    //if we are before range needed
                    if (element == faceRange)
                    {
                        continue;
                    }

                    if (element.First + element.Length < faceRange.First)
                    {
                        continue;
                    }

                    // if we are after range needed
                    if (element.First >= faceRange.First + faceRange.Length)
                    {
                        break;
                    }

                    // if face range after element start
                    if (element.First < faceRange.First)
                    {
                        var preFace = new CharacterRange(element.First, faceRange.First - element.First);

                        element.Length -= preFace.Length;
                        element.First += preFace.Length;
                        elements[i] = element;

                        elements.Insert(i++, preFace);
                    }

                    if (element.First + element.Length <= faceRange.First + faceRange.Length)
                    {
                        continue;
                    }

                    var afterOffset = faceRange.First + faceRange.Length;
                    var afterOffsetLength = element.First + element.Length - afterOffset;

                    element.Length -= afterOffsetLength;
                    elements[i] = element;

                    elements.Insert(++i, new CharacterRange(afterOffset, afterOffsetLength));
                }
            }

            var els = new List<StylizedRowElement>();
            foreach (var range in elements)
            {
                var rangeStyle = Style;
                foreach (var face in styles)
                {
                    if (!intersect(face.Range, range))
                        continue;
                    rangeStyle = rangeStyle.Combine(FindFace(face.Face).FaceStyle);
                }

                els.Add(new StylizedRowElement(range, rangeStyle));
            }

            return els.ToArray();
        }

        private static bool intersect(CharacterRange lhs, CharacterRange rhs)
        {
            var min = Math.Max(lhs.First, rhs.First);
            var max = Math.Min(lhs.First + lhs.Length, rhs.First + rhs.Length);
            return min < max;
        }

        private void updateStyle(IStylizer s, int startLine, int endLine)
        {
            var range = new DocumentRange
            {
                Start = new DocumentPoint(startLine, 0),
                End = new DocumentPoint(endLine, Rows[endLine].Length)
            };
            s.Stylize(new StylizerSourceImpl(this, range));
        }

        public void AssignFace(DocumentRange target, string face)
        {
            AssignFace(target, faces[face]);
        }

        private List<FaceApplied> getRowStyles(int row)
        {
            List<FaceApplied> rowStyles;
            if (!stylesApplied.TryGetValue(row, out rowStyles))
            {
                stylesApplied[row] = rowStyles = new List<FaceApplied>();
            }
            return rowStyles;
        }

        public void AssignFace(DocumentRange target, IStyleFace face)
        {
            if (face == null) return;

            var line = target.Start.Line;
            var column = target.Start.Column;

            while (line <= target.End.Line)
            {
                var chRange = line == target.End.Line 
                    ? new CharacterRange(column, target.End.Column - column) 
                    : new CharacterRange(column, Rows[line].Length);

                getRowStyles(line).Add(new FaceApplied(face.Name, chRange));

                fireFaceChanged(Rows[line]);

                column = 0;
                line++;
            }
        }

        internal event DocumentRowEventHandler FaceChanged;

        private void fireFaceChanged(DocumentRow row)
        {
            if (FaceChanged != null)
                FaceChanged(this, row);
        }
    }
}

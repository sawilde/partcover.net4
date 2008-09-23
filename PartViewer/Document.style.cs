using System;
using System.Collections.Generic;
using System.Drawing;

using PartViewer.Model;
using PartViewer.Styles;

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

            public string Face {get {return face;}}
            public CharacterRange Range { get { return range; } }

            public FaceApplied(string face, CharacterRange range)
            {
                this.face = face;
                this.range = range;
            }
        }

        private class StylizerSourceImpl : StylizerSource
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

            public void setFace(DocumentRange target, string face) { doc.setFace(target, face); }
            public void setFace(DocumentRange target, StyleFace face) { doc.setFace(target, face); }
        }

        private class Face : StyleFace
        {
            private readonly string name;
            private readonly Stylizer owner;
            private readonly Style faceStyle;

            public string Name
            {
                get { return name; }
            }

            public Stylizer Owner
            {
                get { return owner; }
            }

            public Style FaceStyle
            {
                get { return faceStyle; }
            }

            public Face(string name, Stylizer owner)
            {
                this.name = name;
                this.owner = owner;
                faceStyle = new Style();
            }
        }

        readonly List<Stylizer> stylizers = new List<Stylizer>();
        readonly Dictionary<int, List<FaceApplied>> stylesApplied = new Dictionary<int, List<FaceApplied>>();
        readonly Dictionary<string, Face> faces = new Dictionary<string, Face>();

        public StyleFace createFace(Stylizer stylizer, string name)
        {
            Face face;
            if (faces.ContainsKey(name))
                return null;

            faces[name] = face = new Face(name, stylizer);
            return face;
        }

        public StyleFace getFace(string name)
        {
            return faces[name];
        }

        public void addStylizer(Stylizer stylizer)
        {
            stylizers.Add(stylizer);
            updateStyle(stylizer, StartPoint.Y, EndPoint.Y);
        }

        public void removeStylizer(Stylizer stylizer)
        {
            stylizers.Remove(stylizer);

            List<string> faceToRemove = new List<string>();
            foreach(KeyValuePair<string, Face> f in faces)
            {
                if (f.Value.Owner == stylizer)
                    faceToRemove.Add(f.Key);
            }

            Dictionary<int, bool> rowList = new Dictionary<int, bool>();
            foreach(string s in faceToRemove) 
            {
                foreach (KeyValuePair<int, List<FaceApplied>> sa in stylesApplied)
                {
                    int removed = sa.Value.RemoveAll(delegate(FaceApplied fa)
                    {
                        return fa.Face.Equals(s);
                    });

                    if (removed > 0)
                    {
                        rowList[sa.Key] = true;
                    }
                }
                faces.Remove(s);
            }

            foreach (KeyValuePair<int, bool> a in rowList)
            {
                if (stylesApplied.ContainsKey(a.Key) && stylesApplied[a.Key].Count == 0)
                    stylesApplied.Remove(a.Key);
            }

            foreach (KeyValuePair<int, bool> a in rowList)
            {
                fireFaceChanged(Rows[a.Key]);
            }
        }

        public void removeStylizers()
        {
            foreach (Stylizer s in Stylizers)
                removeStylizer(s);
        }

        public Stylizer[] Stylizers {
            get { return stylizers.ToArray(); }
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
                StylizedRowElement[] result = new StylizedRowElement[1];
                result[0] = new StylizedRowElement(new CharacterRange(0, row.Length), style);
                return result;
            }

            List<CharacterRange> elements = new List<CharacterRange>();
            elements.Add(new CharacterRange(0, row.Length));

            foreach (FaceApplied styleApplied in styles)
            {
                CharacterRange faceRange = styleApplied.Range;

                for (int i = 0; i < elements.Count; ++i)
                {
                    CharacterRange element = elements[i];

                    //if we are before range needed
                    if (element == faceRange)
                    {
                        continue;
                    }
                    else if (element.First + element.Length < faceRange.First)
                    {
                        continue;
                    }
                    // if we are after range needed
                    else if (element.First >= faceRange.First + faceRange.Length)
                    {
                        break;
                    }

                    // if face range after element start
                    if (element.First < faceRange.First) 
                    {
                        CharacterRange preFace = new CharacterRange(element.First, faceRange.First - element.First);

                        element.Length -= preFace.Length;
                        element.First += preFace.Length;
                        elements[i] = element;

                        elements.Insert(i++, preFace);

                    }

                    if (element.First + element.Length > faceRange.First + faceRange.Length)
                    {
                        int afterOffset = faceRange.First + faceRange.Length;
                        int afterOffsetLength = element.First + element.Length - afterOffset;

                        element.Length -= afterOffsetLength;
                        elements[i] = element;

                        elements.Insert(++i, new CharacterRange(afterOffset, afterOffsetLength));
                    }
                }
            }

            List<StylizedRowElement> els = new List<StylizedRowElement>();
            foreach (CharacterRange range in elements)
            {
                Style rangeStyle = Style;
                foreach (FaceApplied face in styles)
                {
                    if (!intersect(face.Range, range))
                        continue;
                    rangeStyle = rangeStyle.combine(getFace(face.Face).FaceStyle);
                }

                els.Add(new StylizedRowElement(range, rangeStyle));
            }

            return els.ToArray();
        }

        private static bool intersect(CharacterRange lhs, CharacterRange rhs)
        {
            int min = Math.Max(lhs.First, rhs.First);
            int max = Math.Min(lhs.First + lhs.Length, rhs.First + rhs.Length);
            return min < max;
        }

        private void updateStyle(Stylizer s, int startLine, int endLine)
        {
            DocumentRange range = new DocumentRange();
            range.Start = new DocumentPoint(startLine, 0);
            range.End = new DocumentPoint(endLine, Rows[endLine].Length);

            s.stylize(new StylizerSourceImpl(this, range));
        }

        public void setFace(DocumentRange target, string face)
        {
            setFace(target, faces[face]);
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

        public void setFace(DocumentRange target, StyleFace face)
        {
            if (face == null) return;

            int line = target.Start.Line;
            int column = target.Start.Column;

            while (line <= target.End.Line)
            {
                CharacterRange chRange;

                if (line == target.End.Line)
                {
                    chRange = new CharacterRange(column, target.End.Column - column);
                }
                else
                {
                    chRange = new CharacterRange(column, Rows[line].Length);
                }

                getRowStyles(line).Add(new FaceApplied(face.Name, chRange));

                fireFaceChanged(Rows[line]);

                column = 0;
                line++;
            }
        }

        internal event DocumentRowEventHandler FaceChanged;

        private void fireFaceChanged(DocumentRow row) {
            if (FaceChanged != null)
                FaceChanged(this, row);
        }
    }
}

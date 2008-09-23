using System.Drawing;

namespace PartViewer
{
    partial class DocumentView {
        public Point getNearestCaretPosition(Point viewScreenPoint)
        {
            if (Document == null)
                return Point.Empty;

            PointF pt = viewScreenPoint;
            pt.X += renderStuff.LeftOffset;

            DocumentRowView candidate = null;
            for (int o = renderStuff.FirstRow; o <= renderStuff.LastRow && candidate == null; ++o)
            {
                DocumentRowView rowView = renderList[o];
                if (rowView.bounds.Size.Height > pt.Y)
                {
                    candidate = rowView;
                }
                pt.Y -= rowView.bounds.Size.Height;
            }

            if (candidate == null)
            {
                candidate = renderList[renderStuff.LastRow];
            }

            StylizedRowElement atom = null;
            float partOffset = pt.X;
            for (int i = 0; i < candidate.partSizes.Length; ++i)
            {
                if (partOffset < candidate.partSizes[i].Width)
                {
                    atom = candidate.parts[i];
                    break;
                }
                partOffset -= candidate.partSizes[i].Width;
            }

            if (atom == null)
            {
                atom = candidate.parts[candidate.parts.Length - 1];
            }

            Point res = new Point();

            res.Y = candidate.row.Index;
            res.X = findCharacterPositionInPart(candidate.row, atom, pt.X);

            return res;
        }

        private int findCharacterPositionInPart(DocumentRow row, StylizedRowElement atom, float offset)
        {
            int farLength = atom.range.Length;
            int nearLength = 0;

            CharacterRange range = new CharacterRange(0, farLength);
            string test = row.Raw.Substring(range.First, range.Length);

            float nearSize = 0;
            float farSize = MeasureStringPart(range, test, atom.style).Size.Width;

            if (nearSize > offset) return nearLength;
            if (farSize < offset) return farLength - 1;

            while (farLength > nearLength)
            {
                if (farSize < offset)
                {
                    break;
                }

                if (nearSize > offset)
                {
                    farLength = nearLength;
                    break;
                }

                range = new CharacterRange(0, nearLength + (farLength - nearLength) / 2);
                test = row.Raw.Substring(range.First, range.Length);
                float middle = MeasureStringPart(range, test, atom.style).Size.Width;

                if (middle < offset)
                {
                    if (nearLength == range.Length)
                    {
                        return farSize - middle > middle - nearSize ? nearLength : farLength;
                    }

                    nearLength = range.Length;
                    nearSize = middle;
                }
                else
                {
                    if (farLength == range.Length)
                    {
                        return farSize - middle > middle - nearSize ? nearLength : farLength;
                    }

                    farLength = range.Length;
                    farSize = middle;
                }
            }

            return farLength;
        }

        private float findCharacterPositionOffset(DocumentRowView rowView, int letterOffset)
        {
            float result = 0;

            for (int i = 0; i < rowView.parts.Length && letterOffset > 0; ++i)
            {
                StylizedRowElement atom = rowView.parts[i];
                if (atom.range.Length <= letterOffset)
                {
                    result += rowView.partSizes[i].Width;
                }
                else 
                {
                    CharacterRange range = new CharacterRange(0, letterOffset);
                    string test = rowView.row.Raw.Substring(range.First, range.Length);
                    result += MeasureStringPart(range, test, atom.style).Size.Width;
                }
                letterOffset -= atom.range.Length; 
            }

            return result;
        }
    }
}

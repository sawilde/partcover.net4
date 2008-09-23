using System;

namespace PartViewer.Styles
{
    public class ViewStyle
    {
        private int caretPaddingBottom = 2;
        private int caretPaddingTop = 2;

        private float caretPaddingLeft = .1f;
        private float caretPaddingRight = .1f;

        private int tabSize = 4;

        private bool hideInactiveCursor;
        private bool hideInactiveSelection;

        public int CaretPaddingTop
        {
            get { return caretPaddingTop;  }
            set { caretPaddingTop = Math.Max(2, value); }
        }

        public int CaretPaddingBottom
        {
            get { return caretPaddingBottom; }
            set { caretPaddingBottom = Math.Max(2, value); }
        }

        public float CaretPaddingLeft
        {
            get { return caretPaddingLeft; }
            set { caretPaddingLeft = Math.Min(0.45f, value); }
        }

        public float CaretPaddingRight
        {
            get { return caretPaddingRight; }
            set { caretPaddingRight = Math.Min(0.45f, value); }
        }

        public int TabSize
        {
            get { return tabSize; }
            set { tabSize = Math.Min(0, value); }
        }

        public bool HideInactiveCursor
        {
            get { return hideInactiveCursor; }
            set { hideInactiveCursor = value; }
        }

        public bool HideInactiveSelection
        {
            get { return hideInactiveSelection; }
            set { hideInactiveSelection = value; }
        }
    }
}

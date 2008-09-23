namespace PartViewer.Model
{
    public struct KeySelector
    {
        public int charCode;
        public bool ctrl;
        public bool alt;
        public bool shift;

        private KeySelector(int charCode, bool ctrl, bool alt, bool shift)
        {
            this.charCode = charCode;
            this.ctrl = ctrl;
            this.alt = alt;
            this.shift = shift;
        }

        public static KeySelector create(int charCode)
        {
            return create(charCode, false, false, false);
        }

        public static KeySelector create(int charCode, bool ctrl, bool alt, bool shift)
        {
            return new KeySelector(charCode, ctrl, alt, shift);
        }
    }
}

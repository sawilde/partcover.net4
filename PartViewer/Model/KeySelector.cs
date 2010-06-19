namespace PartViewer.Model
{
    public struct KeySelector
    {
        public int CharCode { get; set; }
        public bool Control { get; set; }
        public bool Alt { get; set; }
        public bool Shift { get; set; }

        private KeySelector(int charCode, bool ctrl, bool alt, bool shift)
        {
            this = new KeySelector 
            {
                CharCode = charCode, 
                Control = ctrl, 
                Alt = alt, 
                Shift = shift
            };
        }

        public static KeySelector Create(int charCode)
        {
            return Create(charCode, false, false, false);
        }

        public static KeySelector Create(int charCode, bool ctrl, bool alt, bool shift)
        {
            return new KeySelector(charCode, ctrl, alt, shift);
        }

        public override bool Equals(object obj)
        {
            if (obj is KeySelector)
                return Equals((KeySelector)obj);
            return base.Equals(obj);
        }

        public bool Equals(KeySelector keySelector)
        {
            return keySelector.Alt == Alt &&
                keySelector.CharCode == CharCode &&
                keySelector.Control == Control &&
                keySelector.Shift == Shift;
        }

        public override int GetHashCode()
        {
            return Alt.GetHashCode() + CharCode.GetHashCode() + Control.GetHashCode() + Shift.GetHashCode();
        }

        public static bool operator ==(KeySelector left, KeySelector right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(KeySelector left, KeySelector right)
        {
            return left.Equals(right);
        }
    }
}

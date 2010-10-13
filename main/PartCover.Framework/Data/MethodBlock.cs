namespace PartCover.Framework.Data
{
    public struct Position
    {
        public int Column { get; set; }
        public int Line { get; set; }
    }

    public class MethodBlock
    {
        public int Offset { get; set; }
        public int File { get; set; }
        public int Length { get; set; }
        public int VisitCount { get; set; }
        public Position Start { get; set; }
        public Position End { get; set; }

        public MethodBlock Copy()
        {
            return new MethodBlock
            {
                Offset = Offset,
                File = File,
                Length = Length,
                VisitCount = VisitCount,
                Start = Start,
                End = End
            };
        }
    }
}
namespace PartCover.Framework.Data
{
    public class SkippedEntry
    {
        public SkippedEntry() {
            AssemblyName = string.Empty;
            TypedefName = string.Empty;
        }

        public string AssemblyName { get; set; }
        public string TypedefName { get; set; }
    }
}
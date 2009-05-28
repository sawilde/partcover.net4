namespace PartCover.Framework
{
    public class SessionRunOptions
    {
        public string TargetPath { get; set; }
        public string TargetDirectory { get; set; }
        public string TargetArguments { get; set; }
        public bool RedirectOutput { get; set; }
        public bool DelayClose { get; set; }
        public bool FlattenDomains { get; set; }
    }
}
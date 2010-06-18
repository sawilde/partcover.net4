namespace PartCover.Framework.Data
{
    public class FileEntry
    {
        public int Id { get; set; }
        public string PathUri { get; set; }

        public FileEntry Copy()
        {
            return new FileEntry { Id = Id, PathUri = PathUri };
        }
    }
}
namespace PartCover.Registration
{
    using System.IO;

    /// <summary>
    /// Simple HKCU registration tool for partcover
    /// </summary>
    public class Program
    {
        /// <summary>
        /// Register the DLL supplied or unregister if empty
        /// </summary>
        /// <param name="args"></param>
        public static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                PartCover.Framework.Registration.UnregisterProfilerForUser();
                return;
            }

            var path = new FileInfo(args[0]).DirectoryName;
            path = Path.Combine(path, "PartCover.dll");

            if (!File.Exists(path))
            {
                throw new FileNotFoundException("PartCover COM library not found", path);
            }

            PartCover.Framework.Registration.RegisterProfilerForUser(path);
        }
    }
}

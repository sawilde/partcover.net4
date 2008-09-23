using System.Collections.Generic;
using System.Text;

namespace PartViewer.Utils
{
    internal static class StringSplitter
    {
        public static string[] split(string source)
        {
            List<string> result = new List<string>();

            StringBuilder builder = new StringBuilder();
            foreach(char ch in source)
            {
                if (ch == '\r')
                    continue;

                builder.Append(ch);

                if (ch == '\n')
                {
                    result.Add(builder.ToString());
                    builder.Length = 0;
                }
            }

            return result.ToArray();
        }
    }
}
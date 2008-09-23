using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Windows.Forms;

namespace PartCover.Browser.Resources
{
    internal class ResourceImageList
    {
        private readonly Dictionary<string, int> imageHashtable = new Dictionary<string, int>();
        private readonly ImageList imageList = new ImageList();
        private readonly string resourcePrefix;
        private readonly string resourcePostfix;
        private readonly Color transparencyColor;

        public ResourceImageList(string resourcePrefix)
            : this(resourcePrefix, string.Empty, Color.Empty)
        {
        }

        public ResourceImageList(string resourcePrefix, string resourcePostfix)
            : this(resourcePrefix, resourcePostfix, Color.Empty)
        {
        }

        public ResourceImageList(string resourcePrefix, string resourcePostfix, Color transparencyColor)
        {
            this.resourcePrefix = resourcePrefix;
            this.resourcePostfix = resourcePostfix;
            this.transparencyColor = transparencyColor;
        }

        public ImageList GetImageList() { return imageList; }

        protected int indexOf(string key)
        {
            key = resourcePrefix + key + resourcePostfix;

            int index;
            if (imageHashtable.TryGetValue(key, out index))
                return index;

            Stream imgStream = Assembly.GetExecutingAssembly().GetManifestResourceStream(key);

            Image img = Image.FromStream(imgStream);

            if (transparencyColor.IsEmpty)
            {
                imageList.Images.Add(img);
                index = imageList.Images.Count;
            }
            else
            {
                index = imageList.Images.Add(img, transparencyColor);
            }

            return imageHashtable[key] = index;
        }
    }
}

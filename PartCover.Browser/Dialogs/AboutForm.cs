using System.Windows.Forms;
using System.Globalization;
using System;
using PartCover.Browser.Api;
using PartCover.Framework;

namespace PartCover.Browser
{
    public partial class AboutForm : Form
    {
        public AboutForm()
		{
			InitializeComponent();

            tbText.Text = string.Format(CultureInfo.InvariantCulture,
                "Browser Version: {0}" + Environment.NewLine +
                "Browser API Version: {1}" + Environment.NewLine +
                "Framework Version: {2}",

                typeof(MainForm).Assembly.GetName().Version.ToString(4),
                typeof(IFeature).Assembly.GetName().Version.ToString(4),
                typeof(Connector).Assembly.GetName().Version.ToString(4));
		}
    }
}

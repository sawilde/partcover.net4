using System.Windows.Forms;

namespace PartCover.Browser.Forms
{
    public partial class RunLogReport : Form
    {
        public RunLogReport()
        {
            InitializeComponent();
        }

        public void AttachLog(string log)
        {
            tbText.Text = log;
        }
    }
}

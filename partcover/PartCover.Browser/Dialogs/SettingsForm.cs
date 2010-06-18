using System.Windows.Forms;
using PartCover.Browser.Properties;

namespace PartCover.Browser.Dialogs
{
    public partial class SettingsForm : Form
    {
        public SettingsForm()
        {
            InitializeComponent();

            UpdatePreview();
        }

        private void UpdatePreview()
        {
            SetStyle(lbExample);
            SetStyle(lbGood);
            SetStyle(lbBad);

            lbGood.BackColor = Settings.Default.ViewPaneCoveredBlockBackroung;
            lbBad.BackColor = Settings.Default.ViewPaneUncoveredBlockBackroung;
            cbAllFile.Checked = Settings.Default.HighlightAllFile;
        }

        private static void SetStyle(Control example)
        {
            example.Font = Settings.Default.ViewPaneFont;
            example.BackColor = Settings.Default.ViewPaneBackground;
            example.ForeColor = Settings.Default.ViewPaneForeground;
        }

        private void btnFont_Click(object sender, System.EventArgs e)
        {
            dlgFont.Font = Settings.Default.ViewPaneFont;
            if (dlgFont.ShowDialog(this) != DialogResult.OK)
                return;

            Settings.Default.ViewPaneFont = dlgFont.Font;
            UpdatePreview();
        }

        private void btnDocumentBack_Click(object sender, System.EventArgs e)
        {
            dlgColor.Color = Settings.Default.ViewPaneBackground;
            if (dlgColor.ShowDialog(this) != DialogResult.OK)
                return;

            Settings.Default.ViewPaneBackground = dlgColor.Color;
            UpdatePreview();
        }

        private void btnDocumentFore_Click(object sender, System.EventArgs e)
        {
            dlgColor.Color = Settings.Default.ViewPaneForeground;
            if (dlgColor.ShowDialog(this) != DialogResult.OK)
                return;

            Settings.Default.ViewPaneForeground = dlgColor.Color;
            UpdatePreview();
        }

        private void btnGoodBack_Click(object sender, System.EventArgs e)
        {
            dlgColor.Color = Settings.Default.ViewPaneCoveredBlockBackroung;
            if (dlgColor.ShowDialog(this) != DialogResult.OK)
                return;

            Settings.Default.ViewPaneCoveredBlockBackroung = dlgColor.Color;
            UpdatePreview();

        }

        private void btnBadBack_Click(object sender, System.EventArgs e)
        {
            dlgColor.Color = Settings.Default.ViewPaneUncoveredBlockBackroung;
            if (dlgColor.ShowDialog(this) != DialogResult.OK)
                return;

            Settings.Default.ViewPaneUncoveredBlockBackroung = dlgColor.Color;
            UpdatePreview();
        }

        private void cbAllFile_CheckedChanged(object sender, System.EventArgs e)
        {
            Settings.Default.HighlightAllFile = cbAllFile.Checked;
        }

        private void btnReset_Click(object sender, System.EventArgs e)
        {
            Settings.Default.Reset();
            UpdatePreview();
        }
    }
}
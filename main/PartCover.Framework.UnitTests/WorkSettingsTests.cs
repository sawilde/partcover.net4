using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using NUnit.Framework;

namespace PartCover.Framework.UnitTests
{
    [TestFixture]
    public class WorkSettingsTests
    {
        [Test]
        public void LoadSettingsFile_FullPath()
        {
            // arrange
            var settings = new WorkSettings();
            settings.SettingsFile = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "FullPath.config");

            // act
            settings.ReadSettingsFile();

            // assert
            Assert.AreEqual(@"C:\TEMP\TARGET.EXE", settings.TargetPath);
            Assert.AreEqual(@"C:\TEMP", settings.TargetWorkingDir);
            Assert.AreEqual(@"C:\TEMP\OUTPUT.TXT", settings.FileNameForReport);
        }

        [Test]
        public void LoadSettingsFile_RelPath()
        {
            // arrange
            var newPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\"));
            var settings = new WorkSettings();
            settings.SettingsFile = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "RelPath.config");

            // act
            settings.ReadSettingsFile();

            // assert
            Assert.AreEqual(Path.Combine(newPath, "TARGET.EXE"), Path.GetFullPath(settings.TargetPath));
            Assert.AreEqual(newPath, Path.GetFullPath(settings.TargetWorkingDir));
            Assert.AreEqual(Path.Combine(newPath, "OUTPUT.TXT"), Path.GetFullPath(settings.FileNameForReport));
        }
    }
}

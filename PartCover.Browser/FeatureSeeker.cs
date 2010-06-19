using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;

using log4net;

using PartCover.Browser.Api;

namespace PartCover.Browser
{
    internal class FeatureSeeker
    {
        private static readonly ILog log = LogManager.GetLogger(MethodBase.GetCurrentMethod().DeclaringType);

        public static IFeature[] seek(Assembly asm)
        {
            log.Info("seek for features in " + asm.GetName().FullName);

            List<IFeature> features = new List<IFeature>();
            foreach (Type t in asm.GetTypes())
            {
                if (t.IsClass && typeof(IFeature).IsAssignableFrom(t))
                {
                    features.Add(createFeatureInstance(t));
                }
            }
            return features.ToArray();
        }

        private static IFeature createFeatureInstance(Type t)
        {
            return (IFeature)Activator.CreateInstance(t);
        }
    }
}

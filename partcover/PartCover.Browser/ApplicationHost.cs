using System.Collections.Generic;
using PartCover.Browser.Api;

namespace PartCover.Browser
{
    internal class ApplicationHost : IServiceContainer
    {
        private readonly List<object> services = new List<object>();

        public T getService<T>() where T : class
        {
            lock (services)
            {
                foreach (object o in services)
                {
                    if (typeof(T).IsInstanceOfType(o))
                        return (T)o;
                }
            }
            return null;
        }

        public bool registerService<T>(T service) where T : class
        {
            lock (services)
            {
                if (services.Contains(service))
                    return false;

                services.Add(service);
                return true;
            }
        }

        public bool unregisterService<T>(T service) where T : class
        {
            lock (services)
            {
                return 0 < services.RemoveAll(actual => (service == actual));
            }
        }

        public void build()
        {
            lock (services)
            {
                foreach (object o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).Attach(this);
                }

                foreach (object o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).Build(this);
                }
            }
        }

        public void destroy()
        {
            lock (services)
            {
                foreach (var o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).Destroy(this);
                }

                foreach (var o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).Detach(this);
                }
                services.Clear();
            }
        }
    }
}

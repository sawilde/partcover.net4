using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Browser.Api;

namespace PartCover.Browser
{
    internal class ApplicationHost : IServiceContainer
    {
        private List<object> services = new List<object>();

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
                return 0 < services.RemoveAll(delegate(object actual)
                {
                    return (service == actual);
                });
            }
        }

        public void build()
        {
            lock (services)
            {
                foreach (object o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).attach(this);
                }

                foreach (object o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).build(this);
                }
            }
        }

        public void destroy()
        {
            lock (services)
            {
                foreach (object o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).destroy(this);
                }

                foreach (object o in services.ToArray())
                {
                    if (o is IFeature) ((IFeature)o).detach(this);
                }
                services.Clear();
            }
        }
    }
}

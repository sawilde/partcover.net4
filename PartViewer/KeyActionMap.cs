using System.Collections;
using System.Collections.Generic;
using PartViewer.Model;

namespace PartViewer
{
    public class KeyActionMap : IEnumerable<KeyValuePair<KeySelector, List<KeyActionHandler>>>
    {
        private readonly Dictionary<KeySelector, List<KeyActionHandler>> handlers;

        public KeyActionMap()
        {
            handlers = new Dictionary<KeySelector, List<KeyActionHandler>>();
        }

        public bool have(KeySelector keys)
        {
            return handlers.ContainsKey(keys);
        }

        public void add(KeySelector keys, KeyActionHandler handler)
        {
            List<KeyActionHandler> list;
            if (!handlers.TryGetValue(keys, out list))
            {
                handlers[keys] = list = new List<KeyActionHandler>();
            }
            list.Add(handler);
        }

        public void add(KeySelector key, IEnumerable<KeyActionHandler> values)
        {
            foreach (KeyActionHandler h in values)
                add(key, h);
        }


        public void remove(KeySelector keys, KeyActionHandler handler)
        {
            List<KeyActionHandler> list;
            if (!handlers.TryGetValue(keys, out list))
                return;

            list.Remove(handler);

            if (list.Count == 0)
                handlers.Remove(keys);
        }

        public void remove(KeySelector key, IEnumerable<KeyActionHandler> values)
        {
            foreach (KeyActionHandler h in values)
                remove(key, h);
        }

        public void execute(KeySelector keys, ActionKeyKind kind)
        {
            List<KeyActionHandler> list;
            if (!handlers.TryGetValue(keys, out list))
                return;
            foreach (KeyActionHandler action in new List<KeyActionHandler>(list))
                action(kind);
        }

        public IEnumerator<KeyValuePair<KeySelector, List<KeyActionHandler>>> GetEnumerator()
        {
            return handlers.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}

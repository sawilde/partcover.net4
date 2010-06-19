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

        public bool Contains(KeySelector keys)
        {
            return handlers.ContainsKey(keys);
        }

        public void Add(KeySelector keys, KeyActionHandler handler)
        {
            List<KeyActionHandler> list;
            if (!handlers.TryGetValue(keys, out list))
            {
                handlers[keys] = list = new List<KeyActionHandler>();
            }
            list.Add(handler);
        }

        public void Add(KeySelector key, IEnumerable<KeyActionHandler> values)
        {
            foreach (var h in values)
                Add(key, h);
        }


        public void Remove(KeySelector keys, KeyActionHandler handler)
        {
            List<KeyActionHandler> list;
            if (!handlers.TryGetValue(keys, out list))
                return;

            list.Remove(handler);

            if (list.Count == 0)
                handlers.Remove(keys);
        }

        public void Remove(KeySelector key, IEnumerable<KeyActionHandler> values)
        {
            foreach (var h in values)
                Remove(key, h);
        }

        public void Execute(KeySelector keys, ActionKeyKind kind)
        {
            List<KeyActionHandler> list;
            if (!handlers.TryGetValue(keys, out list))
                return;
            foreach (var action in new List<KeyActionHandler>(list))
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

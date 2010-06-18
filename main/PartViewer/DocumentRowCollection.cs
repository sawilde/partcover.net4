using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

namespace PartViewer
{
    [DebuggerTypeProxy(typeof(DebugView))]
    public class DocumentRowCollection : IEnumerable<DocumentRow>
    {
        #region DebugView
        internal sealed class DebugView
        {
            private readonly DocumentRowCollection collection;

            public DebugView(DocumentRowCollection collection)
            {
                this.collection = collection;
            }

            [DebuggerBrowsable(DebuggerBrowsableState.RootHidden)]
            public DocumentRow[] Items
            {
                get { return collection.data.ToArray(); }
            }
        }

        
        public int Count
        {
            [DebuggerHidden]
            get { lock (data) return data.Count; }
        }

        #endregion DebugView

        #region IEnumerable
        public IEnumerator<DocumentRow> GetEnumerator()
        {
            return data.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
        #endregion IEnumerable

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        readonly List<DocumentRow> data = new List<DocumentRow>();

        public void Add(string[] value)
        {
            Add(Array.ConvertAll<string, DocumentRow>(value, DocumentRow.create));
        }

        private void Add(DocumentRow[] value)
        {
            lock (data)
            {
                for (var i = 0; i < value.Length; ++i)
                    value[i].Index = data.Count + i;
                data.AddRange(value);
            }
        }

        public DocumentRow this[int index]
        {
            [DebuggerHidden]
            get {
                lock (data)
                {
                    return index >= 0 && index < data.Count ? data[index] : null;
                }
            }
        }
    }
}
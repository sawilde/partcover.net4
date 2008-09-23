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

        public void add(string[] strings)
        {
            add(Array.ConvertAll<string, DocumentRow>(strings, DocumentRow.create));
        }

        private void add(DocumentRow[] rows)
        {
            lock (data)
            {
                for (int i = 0; i < rows.Length; ++i)
                    rows[i].Index = data.Count + i;
                data.AddRange(rows);
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
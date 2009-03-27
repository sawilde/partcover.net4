using System;

namespace PartCover.Framework
{
    public class EventArgs<T> : EventArgs
    {
        private readonly T data;

        public T Data { get { return data; } }

        public EventArgs(T data) { this.data = data; }
    }
}

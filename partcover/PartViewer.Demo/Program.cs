using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace PartViewer.Demo
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Demo());
        }
    }

    public class Test
    {
        int prop;

        public int Property1
        {
            get { return prop; }
            set { prop = value; }
        }

        public int Property2
        {
            internal get { return 0; }
            set { prop = value; }
        }

        public int Property4
        {
            protected get { return 0; }
            set { prop = value; }
        }

        public int Property5
        {
            internal protected get { return 0; }
            set { prop = value; }
        }

        public int Property6
        {
            private get { return 0; }
            set { prop = value; }
        }

        public int Property7
        {
            get { return prop; }
            internal set { prop = value; }
        }

        public int Property8
        {
            get { return prop; }
            protected set { prop = value; }
        }

        public int Property9
        {
            get { return prop; }
            internal protected set { prop = value; }
        }

        public int Property10
        {
            get { return prop; }
            private set { prop = value; }
        }
    }

    public class Test2
    {
        int prop;

        protected int Property1
        {
            get { return prop; }
            set { prop = value; }
        }

        protected int Property6
        {
            private get { return 0; }
            set { prop = value; }
        }

        protected int Property10
        {
            get { return prop; }
            private set { prop = value; }
        }
    }

    public class Test3
    {
        int prop;

        private int Property1
        {
            get { return prop; }
            set { prop = value; }
        }
    }

    public class Test4
    {
        public static implicit operator Test4(int i) { return null; }
    }
}
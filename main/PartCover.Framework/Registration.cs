namespace PartCover.Framework
{
    using System;
    using System.Collections.Generic;
    using System.IO;

    using Microsoft.Win32;

    /// <summary>
    /// Simple wrapper for all the registry keys used by the PartCover COM objects
    /// </summary>
    public sealed class Registration : IDisposable
    {
        /// <summary>
        /// A marker for the filename
        /// </summary>
        private const string Name = "\0";

        /// <summary>
        /// A marker for the containing directory name
        /// </summary>
        private const string Dir = "\u0001";

        /// <summary>
        /// All the registry keys
        /// </summary>
        private static readonly Keys[] RegistryEntries = new[] 
        {
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\AppID\7D0E6AAB-C5FC-4103-AAD4-8BF3112A56C4",
                        Values = new Dictionary<string, string> { { string.Empty, "PartCoverCorDriver" } }
                },
                SubKeys = new List<Key>()
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\AppID\PartCoverCorDriver.DLL",
                        Values = new Dictionary<string, string> { { "AppId", "7d0e6aab-c5fc-4103-aad4-8bf3112a56c4" } }
                },
                SubKeys = new List<Key>()
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\CLSID\{717FF691-2ADF-4AC0-985F-1DD3C42FDF90}",
                        Values = new Dictionary<string, string> { { string.Empty, "CorProfiler Object" }, { "AppId", "7d0e6aab-c5fc-4103-aad4-8bf3112a56c4" } }
                },
                SubKeys = new List<Key> 
                {
                        new Key 
                        { 
                            Name = "InprocServer32", Values = new Dictionary<string, string> { { string.Empty, Name }, { "ThreadingModel", "Both" } } 
                        },
                        new Key 
                        { 
                            Name = "ProgID", Values = new Dictionary<string, string> { { string.Empty, "PartCover.CorDriver.CorProfiler.4" } } 
                        },
                        new Key 
                        { 
                            Name = "Programmable", Values = new Dictionary<string, string>()
                        },
                        new Key 
                        { 
                            Name = "TypeLib", Values = new Dictionary<string, string> { { string.Empty, "{7D0E6AAB-C5FC-4103-AAD4-8BF3112A56C4}" } } 
                        },
                        new Key 
                        { 
                            Name = "VersionIndependentProgID", Values = new Dictionary<string, string> { { string.Empty, "PartCover.CorDriver.CorProfiler" } } 
                        },
                }
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\CLSID\{FB20430E-CDC9-45D7-8453-272268002E08}",
                        Values = new Dictionary<string, string> { { string.Empty, "PartCoverConnector2 Object" }, { "AppId", "7d0e6aab-c5fc-4103-aad4-8bf3112a56c4" } }
                },
                SubKeys = new List<Key> 
                {
                        new Key 
                        { 
                            Name = "InprocServer32", Values = new Dictionary<string, string> { { string.Empty, Name }, { "ThreadingModel", "Both" } } 
                        },
                        new Key 
                        { 
                            Name = "ProgID", Values = new Dictionary<string, string> { { string.Empty, "PartCover.CorDriver.Connector.3" } } 
                        },
                        new Key 
                        { 
                            Name = "Programmable", Values = new Dictionary<string, string>()
                        },
                        new Key 
                        { 
                            Name = "TypeLib", Values = new Dictionary<string, string> { { string.Empty, "{7D0E6AAB-C5FC-4103-AAD4-8BF3112A56C4}" } } 
                        },
                        new Key 
                        { 
                            Name = "VersionIndependentProgID", Values = new Dictionary<string, string> { { string.Empty, "PartCover.CorDriver.Connector" } } 
                        },
                }
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\PartCover.CorDriver.Connector",
                        Values = new Dictionary<string, string> { { string.Empty, "PartCoverConnector2 Object" } }
                },
                SubKeys = new List<Key> 
                {
                        new Key 
                        { 
                            Name = "CLSID", Values = new Dictionary<string, string> { { string.Empty, "{FB20430E-CDC9-45D7-8453-272268002E08}" } } 
                        },
                        new Key 
                        { 
                            Name = "CurVer", Values = new Dictionary<string, string> { { string.Empty, "PartCover.CorDriver.Connector.3" } } 
                        },
                }
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\PartCover.CorDriver.Connector.3",
                        Values = new Dictionary<string, string> { { string.Empty, "PartCoverConnector2 Object" } }
                },
                SubKeys = new List<Key>
                {
                        new Key 
                        { 
                            Name = "CLSID", Values = new Dictionary<string, string> { { string.Empty, "{FB20430E-CDC9-45D7-8453-272268002E08}" } } 
                        },
                }
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\PartCover.CorDriver.CorProfiler",
                        Values = new Dictionary<string, string> { { string.Empty, "CorProfiler Object" } }
                },
                SubKeys = new List<Key> 
                {
                        new Key 
                        { 
                            Name = "CLSID", Values = new Dictionary<string, string> { { string.Empty, "{717FF691-2ADF-4AC0-985F-1DD3C42FDF90}" } } 
                        },
                        new Key 
                        { 
                            Name = "CurVer", Values = new Dictionary<string, string> { { string.Empty, "PartCover.CorDriver.CorProfiler.4" } } 
                        },
                }
            },
            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\PartCover.CorDriver.CorProfiler.4",
                        Values = new Dictionary<string, string> { { string.Empty, "CorProfiler Object" } }
                },
                SubKeys = new List<Key> 
                {
                        new Key 
                        {
                            Name = "CLSID", Values = new Dictionary<string, string> { { string.Empty, "{717FF691-2ADF-4AC0-985F-1DD3C42FDF90}" } } 
                        },
                }
            },

            new Keys 
            {
                Key = new Key 
                {
                        Name = @"Software\Classes\TypeLib\{7d0e6aab-c5fc-4103-aad4-8bf3112a56c4}\4.0",
                        Values = new Dictionary<string, string> { { string.Empty, "PartCover module" } }
                },
                SubKeys = new List<Key> 
                {
                        new Key { Name = @"0\win32", Values = new Dictionary<string, string> { { string.Empty, Name } } },
                        new Key { Name = "FLAGS", Values = new Dictionary<string, string> { { string.Empty, "0" } } },
                        new Key { Name = "HELPDIR", Values = new Dictionary<string, string> { { string.Empty, Dir } } },
                }
            }
        };

        /// <summary>
        /// Initializes a new instance of the Registration class.
        /// </summary>
        /// <param name="assemblyPath">Non-null path of executing assembly -- partcover.dll 
        /// is assumed to be in the same folder</param>
        private Registration(string assemblyPath)
        {
            RegisterProfilerForUser(assemblyPath);
        }

        /// <summary>
        /// Factory method
        /// </summary>
        /// <param name="assemblyPath">Path of executing assembly -- partcover.dll 
        /// is assumed to be in the same folder; null to skip the per-user 
        /// temporary registration</param>
        /// <returns>The disposable to wrap all transactions with</returns>
        public static IDisposable Create(string assemblyPath)
        {
            return string.IsNullOrEmpty(assemblyPath) ?
                (IDisposable)new EmptyDisposable() :
                new Registration(assemblyPath);
        }

        /// <summary>
        /// Registration helper method
        /// </summary>
        /// <param name="assemblyPath">Path of executing assembly -- partcover.dll 
        /// is assumed to be in the same folder; null to skip the per-user 
        /// temporary registration</param>
        public static void RegisterProfilerForUser(string assemblyPath)
        {
            var directory = new FileInfo(assemblyPath).DirectoryName;
            string name = Path.Combine(directory, "PartCover.dll");

            Array.ForEach(
                RegistryEntries,
                x => MakeKey(name, x));            
        }

        /// <summary>
        /// Unregistration helper method -- idempotent delete of the main registry keys
        /// and their subtrees
        /// </summary>
        public static void UnregisterProfilerForUser()
        {
            Array.ForEach(
                RegistryEntries,
                x =>
                {
                    try
                    {
                        Registry.CurrentUser.DeleteSubKeyTree(x.Key.Name);
                    }
                    catch (ArgumentException)
                    {
                        ////Already deleted
                    }
                });
        }

        /// <summary>
        /// Dispose of managed resources only
        /// </summary>
        public void Dispose()
        {
            UnregisterProfilerForUser();
        }

        /// <summary>
        /// Substitute a real file or folder for placeholder values
        /// </summary>
        /// <param name="filename">DLL to register</param>
        /// <param name="value">Dictionary value</param>
        /// <returns>Possibly substituted value</returns>
        private static string ReMap(string filename, string value)
        {
            switch (value)
            {
                case Name:
                    return filename;
                case Dir:
                    return new FileInfo(filename).DirectoryName;
                default:
                    return value;
            }
        }

        /// <summary>
        /// Create a subkey
        /// </summary>
        /// <param name="filename">DLL to register</param>
        /// <param name="root">Relative root key</param>
        /// <param name="key">Description of key and values</param>
        private static void MakeSubKey(string filename, RegistryKey root, Key key)
        {
            using (var regkey = root.CreateSubKey(key.Name))
            {
                foreach (string name in key.Values.Keys)
                {
                    SetValue(filename, regkey, name, key.Values[name]);
                }
            }
        }

        /// <summary>
        /// Create a key and all subkeys
        /// </summary>
        /// <param name="filename">DLL to register</param>
        /// <param name="key">Key set to create</param>
        private static void MakeKey(string filename, Keys key)
        {
            using (var regkey = Registry.CurrentUser.CreateSubKey(key.Key.Name))
            {
                foreach (string name in key.Key.Values.Keys)
                {
                    SetValue(filename, regkey, name, key.Key.Values[name]);
                }

                foreach (var subkey in key.SubKeys)
                {
                    MakeSubKey(filename, regkey, subkey);
                }
            }
        }

        /// <summary>
        /// Set a registry value substituting special placeholder names 
        /// </summary>
        /// <param name="filename">DLL to register</param>
        /// <param name="regkey">Key to add value to</param>
        /// <param name="name">Name to add</param>
        /// <param name="value">Value to add</param>
        private static void SetValue(string filename, RegistryKey regkey, string name, string value)
        {
            regkey.SetValue(string.IsNullOrEmpty(name) ? null : name, ReMap(filename, value));
        }

        /// <summary>
        /// Represents data to make a registry key entry
        /// </summary>
        private struct Key
        {
            /// <summary>
            /// partial path from realtive root
            /// </summary>
            public string Name;

            /// <summary>
            /// All the values (empty -> default)
            /// </summary>
            public IDictionary<string, string> Values;
        }

        /// <summary>
        /// A key and all its subkeys
        /// </summary>
        private struct Keys
        {
            /// <summary>
            /// The root of this subtree
            /// </summary>
            public Key Key;

            /// <summary>
            /// All subordinate keys
            /// </summary>
            public IList<Key> SubKeys;
        }

        /// <summary>
        /// Null disposable
        /// </summary>
        private sealed class EmptyDisposable : IDisposable
        {
            /// <summary>
            /// Does nothing
            /// </summary>
            public void Dispose()
            {
            }
        }
    }
}


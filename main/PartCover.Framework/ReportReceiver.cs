using System;
using System.Reflection;
using PartCover.Framework.Data;
using System.Diagnostics.SymbolStore;

namespace PartCover.Framework
{
    /// <summary>
    /// Receives the results from the profiler and builds a <see cref="Report"/> entity.
    /// </summary>
    /// <remarks>This object is called by the profiler via COM from instrumented_results.cpp</remarks>
    public class ReportReceiver : IReportReceiver, IDisposable
    {
        private AssemblyEntry currentAssembly;
        private TypedefEntry currentTypedef;
        private MethodEntry currentMethod;
        private SymBinder _symBinder;
        private ISymbolReader _symbolReader;
        public Report Report { get; set; }

        public ReportReceiver()
        {
            _symBinder = new SymBinder();
        }

        public void Dispose()
        {
            _symBinder = null;
        }

        /// <summary>
        /// Registers a file in the <see cref="Report.Files"/> collection if it does not already exist
        /// </summary>
        /// <param name="fileUrl"></param>
        private void RegisterFile(string fileUrl)
        {
            if (Report.Files.Exists(x => x.PathUri == fileUrl)) return;
            Report.Files.Add(new FileEntry
            {
                Id = Report.Files.Count,
                PathUri = fileUrl
            });
        }

        /// <summary>
        /// Retrieves an identifier from the <see cref="Report.Files"/> collection if it exists
        /// </summary>
        /// <param name="fileUrl"></param>
        /// <returns></returns>
        private int GetFileIdentifier(string fileUrl)
        {
            var file = Report.Files.Find(x => x.PathUri == fileUrl);
            return (file != null) ? file.Id : 0;
        }

        /// <summary>
        /// Register a skipped item
        /// </summary>
        /// <param name="assemblyName"></param>
        /// <param name="typedefName"></param>
        /// <remarks>Called directly from the </remarks>
        public void RegisterSkippedItem(string assemblyName, string typedefName)
        {
            Report.SkippedItems.Add(new SkippedEntry 
            {
                AssemblyName = assemblyName,
                TypedefName = typedefName
            });
        }

        /// <summary>
        /// Called when receiving a report about an assembly
        /// </summary>
        /// <param name="domainIndex"></param>
        /// <param name="domainName"></param>
        /// <param name="assemblyName"></param>
        /// <param name="moduleName"></param>
        public void EnterAssembly(int domainIndex, string domainName, string assemblyName, string moduleName)
        {
            _symbolReader = SymbolReaderWapper.GetSymbolReader(_symBinder, moduleName);
            if (_symbolReader != null)
            {
                var docs = _symbolReader.GetDocuments();
                foreach (var doc in docs)
                {
                    RegisterFile(doc.URL);
                }
            }

            Report.Assemblies.Add(currentAssembly = new AssemblyEntry
            {
                AssemblyRef = Report.Assemblies.Count + 1,
                Module = moduleName,
                Name = assemblyName,
                Domain = domainName,
                DomainIndex = domainIndex
            });
        }

        /// <summary>
        /// Called when receiving a report about a type
        /// </summary>
        /// <param name="typedefName"></param>
        /// <param name="flags"></param>
        public void EnterTypedef(string typedefName, uint flags)
        {
            currentAssembly.Types.Add(currentTypedef = new TypedefEntry
            {
                Assembly = currentAssembly,
                Name = typedefName,
                Attributes = (TypeAttributes)flags
            });
        }

        /// <summary>
        /// Called when receiving a report about a method
        /// </summary>
        /// <param name="methodName"></param>
        /// <param name="methodSig"></param>
        /// <param name="bodySize"></param>
        /// <param name="flags"></param>
        /// <param name="implFlags"></param>
        /// <param name="symbolFileId"></param>
        /// <param name="methodDef"></param>
        public void EnterMethod(string methodName, string methodSig, int bodySize, uint flags, uint implFlags, int symbolFileId, int methodDef)
        {
            currentTypedef.Methods.Add(currentMethod = new MethodEntry
            {
                Type = currentTypedef,
                Name = methodName,
                Signature = methodSig,
                BodySize = bodySize,
                SymbolFileId = symbolFileId,
                MethodDef = methodDef,
                Flags = (MethodAttributes)flags,
                ImplFlags = (MethodImplAttributes)implFlags
            });

            if (_symbolReader != null)
            {
                var token = new SymbolToken(methodDef);
                ISymbolMethod method;
                try
                {
                    method = _symbolReader.GetMethod(token);
                    var count = method.SequencePointCount;

                    int[] offsets = new int[count];
                    int[] sls = new int[count];
                    int[] scs = new int[count];
                    int[] els = new int[count];
                    int[] ecs = new int[count];
                    ISymbolDocument[] docs = new ISymbolDocument[count];

                    method.GetSequencePoints(offsets, docs, sls, scs, els, ecs);

                    for (int i = 0; i < count; i++)
                    {
                        MethodBlock block = new MethodBlock();
                        block.Offset = offsets[i];
                        var fileId = GetFileIdentifier(docs[i].URL);
                        if (fileId > 0 && sls[i] != 0xFEEFEE)
                        {
                            block.Start = new Position { Column = scs[i], Line = sls[i] };
                            block.End = new Position { Column = ecs[i], Line = els[i] };
                            block.File = fileId;
                        }
                        currentMethod.Blocks.Add(block);
                    }

                    docs = null;
                }
                catch (Exception ex)
                {

                }
                finally
                {
                    method = null;
                }
            }
        }

        /// <summary>
        /// Called when receiving a report about an instrumented block
        /// </summary>
        /// <param name="blockData"></param>
        public void AddCoverageBlock(BLOCK_DATA blockData)
        {
            var block = (currentMethod.Blocks.Count > 0) ? currentMethod.Blocks.Find(x => x.Offset == blockData.position) : null;
            if (block != null)
            {
                //block.Length = blockData.blockLen;
                block.VisitCount = blockData.visitCount;
            }
            else
            {
                currentMethod.Blocks.Add(new MethodBlock
                {
                    File = 0,
                    Offset = blockData.position,
                    //Length = blockData.blockLen,
                    VisitCount = blockData.visitCount,
                    Start = new Position(),
                    End = new Position(),
                });
            }
        }

        /// <summary>
        /// Called when all data regarding a method and it's instrumented block has been delivered
        /// </summary>
        public void LeaveMethod()
        {
            var count = currentMethod.Blocks.Count;
            for (int i = 0; i < count; i++)
            {
                currentMethod.Blocks[i].Length = (i < count - 1)? 
                    currentMethod.Blocks[i + 1].Offset - currentMethod.Blocks[i].Offset : currentMethod.BodySize - currentMethod.Blocks[i].Offset;
            }
        }

        /// <summary>
        /// Called when all data regarding a type and its methods has been delivered
        /// </summary>
        public void LeaveTypedef() { }

        /// <summary>
        /// Called when all data regarding an assembly and its types have been delivered
        /// </summary>
        public void LeaveAssembly() { _symbolReader = null; }
    }
}
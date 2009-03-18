using PartCover.Framework.Walkers;

namespace PartCover.Framework
{
    public class ConnectorActionCallback : IConnectorActionCallback
    {
        private readonly Connector connector;

        public ConnectorActionCallback(Connector connector)
        {
            this.connector = connector;
        }

        void IConnectorActionCallback.DriverConnected() { connector.ProcessCallback.writeStatus("driver connected successfully"); }

        void IConnectorActionCallback.DriverSendRules() { connector.ProcessCallback.writeStatus("send rules to the driver"); }

        void IConnectorActionCallback.DriverWaitEoIConfirm() { connector.ProcessCallback.writeStatus("wait for rules confirm"); }

        void IConnectorActionCallback.FunctionsCount(uint count) { connector.ProcessCallback.writeStatus(string.Format("functions count {0}", count)); }

        void IConnectorActionCallback.FunctionsReceiveBegin() { connector.ProcessCallback.writeStatus("functions loading is being started"); }

        void IConnectorActionCallback.FunctionsReceiveEnd() { connector.ProcessCallback.writeStatus("functions loading is complete"); }

        void IConnectorActionCallback.FunctionsReceiveStat(uint index) { connector.ProcessCallback.writeStatus(string.Format("functions loaded: {0}", index)); }

        void IConnectorActionCallback.InstrumentDataReceiveBegin() { connector.ProcessCallback.writeStatus("instrument data is being received"); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersAsm(string name, string mod, uint typeDefCount) { connector.ProcessCallback.writeStatus(string.Format("load assembly {0} ({1} types)", name, typeDefCount)); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersAsmCount(uint asmCount) { connector.ProcessCallback.writeStatus(string.Format("assembly count: {0}", asmCount)); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersBegin() { connector.ProcessCallback.writeStatus("InstrumentDataReceiveCountersBegin"); }

        void IConnectorActionCallback.InstrumentDataReceiveCountersEnd() { connector.ProcessCallback.writeStatus("InstrumentDataReceiveCountersEnd"); }

        void IConnectorActionCallback.InstrumentDataReceiveEnd() { connector.ProcessCallback.writeStatus("instrument data load complete"); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesBegin() { connector.ProcessCallback.writeStatus("file list is being received"); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesCount(uint fileCount) { connector.ProcessCallback.writeStatus(string.Format("{0} files to load", fileCount)); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesEnd() { connector.ProcessCallback.writeStatus("file list load is complete"); }

        void IConnectorActionCallback.InstrumentDataReceiveFilesStat(uint index) { }

        void IConnectorActionCallback.InstrumentDataReceiveStatus() { connector.ProcessCallback.writeStatus("driver connected"); }

        void IConnectorActionCallback.MethodsReceiveBegin() { connector.ProcessCallback.writeStatus("function map is being received"); }

        void IConnectorActionCallback.MethodsReceiveEnd() { connector.ProcessCallback.writeStatus("function map load complete"); }

        void IConnectorActionCallback.MethodsReceiveStatus() { connector.ProcessCallback.writeStatus("function map load status"); }

        void IConnectorActionCallback.OpenMessagePipe() { connector.ProcessCallback.writeStatus("open driver pipe"); }

        void IConnectorActionCallback.SetConnected(bool connected) { connector.ProcessCallback.writeStatus(connected ? "driver connected" : "driver disconnected"); }

        void IConnectorActionCallback.TargetCreateProcess() { connector.ProcessCallback.writeStatus("create target process"); }

        void IConnectorActionCallback.TargetRequestShutdown() { connector.ProcessCallback.writeStatus("request target shutdown"); }

        void IConnectorActionCallback.TargetSetEnvironmentVars() { connector.ProcessCallback.writeStatus("modify target environment variables"); }

        void IConnectorActionCallback.TargetWaitDriver() { connector.ProcessCallback.writeStatus("wait for driver connection"); }

        void IConnectorActionCallback.LogMessage(int threadId, int tick, string text)
        {
            var message = new CoverageReport.RunLogMessage 
            {
                Message = text, 
                MsOffset = tick, 
                ThreadId = threadId
            };
            connector.OnLogMessage(message);
        }
    }
}

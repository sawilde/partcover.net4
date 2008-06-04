using System;

namespace PartCover.Framework.Walkers
{
	public class ReportException : System.ApplicationException
	{
		public ReportException(string message) : base(message) {}
	}
}

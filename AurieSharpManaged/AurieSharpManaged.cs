using System.Diagnostics;
using System.Runtime.Versioning;
using Aurie.Managed;
using YYTK.Managed;

namespace AurieSharpManaged
{
	[SupportedOSPlatform("windows")]
	public static class AurieSharpManaged
	{
		private static IYYToolkit? m_YYTK = null;
		private static Stopwatch m_Stopwatch = new Stopwatch();

		public static void TestCallback(FWFrame Context)
		{
			if (!m_Stopwatch.IsRunning)
			{
				// Start if not running
				m_Stopwatch.Start();
			}
			else
			{
				m_Stopwatch.Stop();

				string text = "Last frame took {0:F2}ms ({1:F2} fps)";
				text = String.Format(
					text, 
					m_Stopwatch.Elapsed.TotalMicroseconds / 1000.0, 
					(double)(1e6) / m_Stopwatch.Elapsed.TotalMicroseconds
				);

				m_YYTK?.PrintWarning(text);

				m_Stopwatch.Reset();
			}
		}
		
		public static AurieStatus ModuleInitialize()
		{
			m_YYTK = new();
			m_YYTK.PrintWarning("This is from my managed module!!!1");
			m_YYTK.CreateFrameCallback(
				TestCallback
			);

			

			return AurieStatus.Success;
		}
	}
}
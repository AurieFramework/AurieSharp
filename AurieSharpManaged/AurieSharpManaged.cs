using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using Aurie.Managed;
using YYTK.Managed;

namespace AurieSharpManaged
{
	[SupportedOSPlatform("windows")]
	public static class AurieSharpManaged
	{
		public static IYYToolkit? m_YYTK = null;

        private static void TestCallback(UIntPtr Context)
        {
            FWFrame my_event = new(Context);
			var x = my_event.GetSwapchainPointer();
            m_YYTK?.PrintWarning($"Current running code entry: {x}");
        }

        [UnmanagedCallersOnly]
		private static AurieStatus __AurieFrameworkDispatch(
			/* IN */ IntPtr FunctionPointer
		)
		{
			if (FunctionPointer != IntPtr.Zero) 
			{
				AurieEntryDelegate entry = Marshal.GetDelegateForFunctionPointer<AurieEntryDelegate>(
					FunctionPointer
				);

				return entry();
			}

			return AurieStatus.Success;
		}

        private static AurieStatus ModuleInitialize()
        {
			m_YYTK = new();
			m_YYTK.PrintWarning("This is from my managed module!!!1");
            m_YYTK.CreateCallback(EventTriggers.EVENT_FRAME, TestCallback, 0);

            return AurieStatus.Success;
        }

    }
}
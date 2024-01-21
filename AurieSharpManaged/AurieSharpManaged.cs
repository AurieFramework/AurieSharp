using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using Aurie.Managed;
using YYTK.Managed;

namespace AurieSharpManaged
{
	[SupportedOSPlatform("windows")]
	public static class AurieSharpManaged
	{
		public delegate AurieStatus AurieEntryDelegate();

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
			IYYToolkit yytk_interface = new();

			yytk_interface.PrintWarning("This is from my managed module!!!1");

			// show_debug_overlay(true)
            yytk_interface.CallBuiltin("show_debug_overlay", new List<RValue>() { new RValue(true) });
			
			yytk_interface.PrintWarning("Debug overlay should be enabled!");

            return AurieStatus.Success;
		}
	}
}
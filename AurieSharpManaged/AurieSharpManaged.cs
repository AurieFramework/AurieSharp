using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using AurieSharp;

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
			bool suspended = true;

			short major = 0, minor = 0, patch = 0;
            AurieInterface.MmGetFrameworkVersion(ref major, ref minor, ref patch);

			AurieInterface.ElIsProcessSuspended(ref suspended);

			if (!suspended)
			{
                Console.Beep(1000, 1000);
                return AurieStatus.Success;
            }

			return AurieStatus.ModuleDependencyNotResolved;
		}
	}
}
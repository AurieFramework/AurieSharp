using SharpToolkit;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace Aurie
{
	[SupportedOSPlatform("windows")]
	public static class AurieSharpManaged
	{
		public delegate AurieStatus AurieEntryDelegate(IAurie Interface);

		[UnmanagedCallersOnly]
		private static AurieStatus __AurieFrameworkDispatch(
			/* IN */ IntPtr Module,
			/* IN */ IntPtr FunctionPointer,
			/* IN */ IntPtr GetFrameworkRoutine
		)
		{
			if (FunctionPointer != IntPtr.Zero) 
			{
				AurieEntryDelegate entry = Marshal.GetDelegateForFunctionPointer<AurieEntryDelegate>(
					FunctionPointer
				);

				return entry(new IAurie(GetFrameworkRoutine));
			}
			
			return AurieStatus.Success;
		}

		private static AurieStatus ModulePreinitialize(IAurie Aurie)
		{
			return AurieStatus.Success;
		}

		private static AurieStatus ModuleInitialize(IAurie Aurie)
		{
			ISharpToolkit sharpToolkit = new ISharpToolkit(Aurie);
			sharpToolkit.Print(ConsoleColor.Blue, "Print()");
            sharpToolkit.PrintInfo("PrintInfo()");
            sharpToolkit.PrintWarning("PrintWarning()");
			sharpToolkit.PrintError("AurieSharpManaged.cs", 44, "PrintError()");

            return AurieStatus.Success;
		}
	}
}
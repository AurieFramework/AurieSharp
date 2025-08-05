using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Runtime.Versioning;
using AurieSharpInterop;
using YYTKInterop;

namespace AurieSharpManaged
{
	[SupportedOSPlatform("windows")]
	public static class AurieSharpManaged
	{
		// Is this assembly AurieSharpManaged?
		private static bool IsAssemblyASM(string AssemblyPath)
		{
			AssemblyLoadContext load_context = new("AurieManagedModContext", false);
			Assembly? assembly = null;

			try
			{
				assembly = load_context.LoadFromAssemblyPath(AssemblyPath);
			}
			catch (Exception ex)
			{
				Debug.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load managed assembly {AssemblyPath} - {ex.Message}");
				return false;
			}

			return assembly.GetTypes().Any(t => t.IsClass && t.IsPublic && t.IsAbstract && t.IsSealed && t.Name == "AurieSharpManaged");
		}

		private static AurieStatus LoadInitializeManagedAssembly(string AssemblyPath)
		{
			AssemblyLoadContext load_context = new("AurieManagedModContext", true);
			Assembly? assembly = null;
			try
			{
				assembly = load_context.LoadFromAssemblyPath(AssemblyPath);
			}
			catch (Exception ex)
			{
				Debug.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load managed assembly {AssemblyPath} - {ex.Message}");
				return AurieStatus.UnknownError;
			}

			// Loop all defined types in the assembly
			foreach (var type in assembly.GetTypes())
			{
				// Look for a public, static InitializeMod method.
				var method = type.GetMethod("InitializeMod", BindingFlags.Static | BindingFlags.Public);

				// If the method either:
				// - Does not exist
				// - Does not return AurieStatus (the managed one)
				// - Takes any parameters (which it shouldn't)
				// it is invalid, and a print should be issued.
				if (method == null || method.ReturnType != typeof(AurieStatus) || method.GetParameters().Length != 0)
				{
					Debug.PrintEx(
						AurieLogSeverity.Warning,
						$"[ASM] Assembly {assembly.GetName()} contains an InitializeMod method, but does not meet required criteria!"
					);

					continue;
				}

				method.Invoke(null, null);
				Debug.PrintEx(
					AurieLogSeverity.Trace,
					$"[ASM] Assembly {assembly.GetName()} is being initialized."
				);

				return AurieStatus.Success;
			}

			// No InitializeMod method exists
			return AurieStatus.InvalidSignature;
		}

		[UnmanagedCallersOnly]
		public static AurieStatus ModuleInitialize()
		{
			string mod_folder = Path.Combine(Framework.GetGameDirectory(), "mods", "Managed");
			Debug.PrintEx(AurieLogSeverity.Trace, $"[ASM] Proceeding to load from {mod_folder}");
			foreach (string file in Directory.GetFiles(mod_folder))
			{
				string extension = Path.GetExtension(mod_folder);
				if (extension != ".dll")
					continue;

				// Skip ourselves (AurieSharpManaged)
				if (IsAssemblyASM(file))
					continue;

				AurieStatus load_status = LoadInitializeManagedAssembly(file);
				if (load_status != AurieStatus.Success)
				{
					Debug.PrintEx(
						AurieLogSeverity.Error,
						$"[ASM] Assembly \"{file}\" could not be loaded - {load_status.ToString()}!"
					);
				}

				Debug.PrintEx(
					AurieLogSeverity.Trace,
					$"[ASM] Loaded file \"{file}\"."
				);
			}


			return AurieStatus.Success;
		}
	}
}
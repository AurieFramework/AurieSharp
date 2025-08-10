using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Runtime.Versioning;
using AurieSharpInterop;

namespace AurieSharpManaged
{
    [SupportedOSPlatform("windows")]
    public static class AurieSharpManaged
    {
        private static List<ManagedMod> m_LoadedMods = new();
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

        [UnmanagedCallersOnly]
        public static AurieStatus ModuleInitialize()
        {
            string mod_folder = Path.Combine(Framework.GetGameDirectory(), "mods", "Managed");
            Debug.PrintEx(AurieLogSeverity.Trace, $"[ASM] Proceeding to load from {mod_folder}");
            foreach (string file in Directory.GetFiles(mod_folder))
            {
                // Get the file extension, including the ending dot.
                string extension = Path.GetExtension(file);

                // Skip files that don't end in .DLL
                if (extension.ToLower() != ".dll")
                    continue;

                // Skip ourselves (AurieSharpManaged)
                if (IsAssemblyASM(file))
                    continue;

                // Create a new mod entry
                ManagedMod new_mod = new(file);

                // Try to actually load the mod
                AurieStatus load_status = new_mod.Load();
                if (load_status != AurieStatus.Success)
                {
                    Debug.PrintEx(
                        AurieLogSeverity.Error,
                        $"[ASM] Assembly \"{file}\" could not be loaded - {load_status.ToString()}!"
                    );
                }

                // Add the mod to our list of loaded modles
                m_LoadedMods.Add(new_mod);

                Debug.PrintEx(
                    AurieLogSeverity.Trace,
                    $"[ASM] Loaded file \"{file}\"."
                );
            }

            return AurieStatus.Success;
        }
        
        [UnmanagedCallersOnly]
        public static AurieStatus ModuleUnload()
        {
            Debug.Print("[ASM] ModuleUnload called - all managed mods will be unloaded.");

            foreach (var mod in m_LoadedMods)
            {
                if (mod.Loaded)
                    mod.Unload(true);
            }

            m_LoadedMods.Clear();

            return AurieStatus.Success;
        }
    }
}
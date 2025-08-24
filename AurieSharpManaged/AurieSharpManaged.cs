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
        private static FileSystemWatcher m_FsWatcher = new();
        // Is this assembly AurieSharpManaged?
        private static bool IsAssemblyASM(string AssemblyPath)
        {
            FileStream fs;
            try
            {
                fs = File.Open(AssemblyPath, FileMode.Open, FileAccess.ReadWrite, FileShare.ReadWrite | FileShare.Delete);
            }
            catch (Exception ex)
            {
                // Return true on exception to not load the mod.
                Framework.PrintEx(AurieLogSeverity.Trace, $"[ASM] Failed to open assembly {AssemblyPath} - {ex.Message}");
                return true;
            }

            AssemblyLoadContext load_context = new("AurieManagedModContext", true);
            Assembly? assembly = null;

            try
            {
                assembly = load_context.LoadFromStream(fs);
            }
            catch (Exception ex)
            {
                // Return true on exception to not load the mod.
                Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load managed assembly {AssemblyPath} - {ex.Message}");
                return true;
            }

            bool is_asm = assembly.GetTypes().Any(t => t.IsClass && t.IsPublic && t.IsAbstract && t.IsSealed && t.Name == "AurieSharpManaged");
            load_context.Unload();

            GC.Collect();
            return is_asm;
        }
        private static void OnModDirectoryFileChange(object sender, FileSystemEventArgs e)
        {
            // Skip ourselves (AurieSharpManaged)
            if (IsAssemblyASM(e.FullPath))
                return;

            GC.Collect();

            ManagedMod? changed_mod = m_LoadedMods.Find((mod) => { return Path.GetFullPath(mod.Path) == e.FullPath; });
            AurieStatus last_status = AurieStatus.Success;

            Framework.PrintEx(AurieLogSeverity.Debug, $"Event {e.ChangeType} occurred for {e.Name}");

            if (e.ChangeType != WatcherChangeTypes.Changed)
                return;

            // If a file was "changed" but is not loaded, then it is a new mod being loaded by a file placed in the mod directory.
            if (changed_mod is null)
            {
                Framework.PrintEx(AurieLogSeverity.Info, $"[ASM] Loading mod {e.Name}...");
                ManagedMod created_mod = new(e.FullPath);

                last_status = created_mod.Load();
                if (last_status != AurieStatus.Success)
                {
                    Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load {e.Name} with status {last_status.ToString()}");
                    created_mod.Unload(false);

                    GC.Collect();
                    return;
                }

                m_LoadedMods.Add(created_mod);
                return;
            }

            Framework.PrintEx(AurieLogSeverity.Info, $"[ASM] Hot-reloading mod {e.Name}");

            // Otherwise, the mod is already loaded, and just needs to be unloaded.
            changed_mod.Unload(true);
            m_LoadedMods.Remove(changed_mod);
            changed_mod = null;

            // Collect the old mod (this frees the memory occuppied by it)
            GC.Collect();

            // Create a new one
            ManagedMod hotreloaded_mod = new(e.FullPath);
            last_status = hotreloaded_mod.Load();

            if (last_status != AurieStatus.Success)
            {
                Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load {e.Name} with status {last_status.ToString()}");
                hotreloaded_mod.Unload(false);

                GC.Collect();
                return;
            }

            m_LoadedMods.Add(hotreloaded_mod);
        }

        private static void OnModDirectoryFileDelete(object sender, FileSystemEventArgs e)
        {
            GC.Collect();

            ManagedMod? changed_mod = m_LoadedMods.Find((mod) => { return Path.GetFullPath(mod.Path) == e.FullPath; });
            Framework.PrintEx(AurieLogSeverity.Debug, $"Event {e.ChangeType} occurred for {e.Name}");

            if (e.ChangeType == WatcherChangeTypes.Deleted)
            {
                // If a file was deleted, but not loaded, we don't care.
                if (changed_mod is null)
                    return;

                changed_mod.Unload(true);
                Framework.PrintEx(AurieLogSeverity.Info, $"[ASM] Unloading mod {e.Name}");

                m_LoadedMods.Remove(changed_mod);
                GC.Collect();

                return;
            }
        }

        [UnmanagedCallersOnly]
        public static AurieStatus ModuleInitialize()
        {
            string mod_folder = Path.Combine(Framework.GetGameDirectory(), "mods", "Managed");
            Framework.PrintEx(AurieLogSeverity.Trace, $"[ASM] Proceeding to load from {mod_folder}");
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
                    Framework.PrintEx(
                        AurieLogSeverity.Error,
                        $"[ASM] Assembly \"{file}\" could not be loaded - {load_status.ToString()}!"
                    );

                    continue;
                }

                // Add the mod to our list of loaded modles
                m_LoadedMods.Add(new_mod);

                Framework.PrintEx(
                    AurieLogSeverity.Trace,
                    $"[ASM] Loaded file \"{file}\"."
                );
            }

            m_FsWatcher.Path = mod_folder;
            m_FsWatcher.Changed += OnModDirectoryFileChange;
            m_FsWatcher.Deleted += OnModDirectoryFileDelete;
            m_FsWatcher.Filter = "*.dll";
            m_FsWatcher.EnableRaisingEvents = true;

            return AurieStatus.Success;
        }

        [UnmanagedCallersOnly]
        public static AurieStatus ModuleUnload()
        {
            Framework.Print("[ASM] ModuleUnload called - all managed mods will be unloaded.");

            m_LoadedMods.ForEach((mod) => { mod.Unload(true); });
            GC.Collect();

            return AurieStatus.Success;
        }
    }
}
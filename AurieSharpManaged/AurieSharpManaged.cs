using System.Reflection;
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
        private static List<ManagedMod> m_LoadedMods = new();
        private static FileSystemWatcher m_FsWatcher = new();
        // Is this assembly AurieSharpManaged?
        private static bool IsAssemblyASM(string AssemblyPath)
        {
            FileStream fs;
            try
            {
                fs = File.Open(AssemblyPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete);
            }
            catch (Exception ex)
            {
                // Return true on exception to not load the mod.
                Framework.PrintEx(AurieLogSeverity.Warning, $"[ASM] IsAssemblyASM fails to open assembly {AssemblyPath} - {ex.Message}");
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
                Framework.PrintEx(AurieLogSeverity.Warning, $"[ASM] IsAssemblyASM fails to load managed assembly {AssemblyPath} - {ex.Message}");
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

                if (!CheckDependenciesResolvable(created_mod))
                {
                    Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load {e.Name} - missing dependencies.");
                    return;
                }

                last_status = created_mod.Load(m_LoadedMods);
                if (last_status != AurieStatus.Success)
                {
                    Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load {e.Name} with status {last_status.ToString()}");
                    UnloadMod(created_mod, false);
                    return;
                }

                m_LoadedMods.Add(created_mod);
                return;
            }

            Framework.PrintEx(AurieLogSeverity.Info, $"[ASM] Hot-reloading mod {e.Name}");

            // Otherwise, the mod is already loaded, and just needs to be unloaded.
            UnloadMod(changed_mod, true);
            changed_mod.Unload(true);
            changed_mod = null;

            // Create a new one
            ManagedMod hotreloaded_mod = new(e.FullPath);

            if (!CheckDependenciesResolvable(hotreloaded_mod))
            {
                Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load {e.Name} - missing dependencies.");
                return;
            }

            last_status = hotreloaded_mod.Load();

            // .Load() automatically cleans up if it fails loading
            if (last_status != AurieStatus.Success)
            {
                Framework.PrintEx(AurieLogSeverity.Error, $"[ASM] Failed to load {e.Name} with status {last_status.ToString()}");
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

                UnloadMod(changed_mod, true);
                Framework.PrintEx(AurieLogSeverity.Info, $"[ASM] Unloading mod {e.Name}");
                return;
            }
        }

        private static void UnloadMod(ManagedMod mod, bool notify)
        {
            // If there are mods that depend on this mod, unload them first (recursively).
            foreach (ManagedMod dep in m_LoadedMods)
            {
                if (dep.Dependencies == null)
                {
                    continue;
                }
                if (dep.Dependencies.Select(d => d.FullName).Contains(mod.AssemblyName!.FullName))
                {
                    UnloadMod(dep, notify);
                }
            }
            mod.Unload(notify);
            m_LoadedMods.Remove(mod);
            GC.Collect();
        }

        private static bool CheckDependenciesResolvable(ManagedMod mod, List<ManagedMod>? potential_mods = null)
        {
            potential_mods ??= [];
            List<AssemblyName> missingDeps = mod.Dependencies!.Where(
                dependency =>
                !m_LoadedMods.Select(i => i.AssemblyName!.FullName).Contains(dependency.FullName)
                && !potential_mods.Select(i => i.AssemblyName!.FullName).Contains(dependency.FullName)).ToList();
            if (missingDeps.Count > 0)
            {
                Framework.PrintEx(
                    AurieLogSeverity.Error,
                    $"[ASM] Assembly \"{mod.Path}\" could not be loaded - missing dependencies: {missingDeps.Select(d => d.Name!).Aggregate((a, b) => $"{a} | {b}").ToString()}"
                    );
                return false;
            }

            return true;
        }

        [UnmanagedCallersOnly]
        public static AurieStatus ModuleInitialize()
        {
            string mod_folder = Path.Combine(Framework.GetGameDirectory(), "mods", "Managed");
            Framework.PrintEx(AurieLogSeverity.Trace, $"[ASM] Proceeding to load from {mod_folder}");
            List<ManagedMod> potential_mods = new();
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

                potential_mods.Add(new_mod);
            }

            //Check if we are missing any dependencies

            //Resolve mod load order for dependencies
            while (potential_mods.Count > 0)
            {
                foreach (ManagedMod mod in potential_mods.ToList())
                {
                    if (!CheckDependenciesResolvable(mod, potential_mods))
                    {
                        // If there are any dependencies missing from both resolved and potential mods, we cannot load this mod at all.
                        potential_mods.Remove(mod);
                        continue;
                    }

                    //If the mod has no dependencies, or all dependencies are already resolved, we can load it.
                    if (mod.Dependencies == null
                        || mod.Dependencies.Count == 0
                        || mod.Dependencies.Select(m => m.FullName).Intersect(m_LoadedMods.Select(i => i.AssemblyName!.FullName)).Count() == mod.Dependencies.Count)
                    {
                        potential_mods.Remove(mod);
                        // Try to actually load the mod
                        AurieStatus load_status = mod.Load(m_LoadedMods);

                        // .Load() automatically cleans up if it fails loading
                        if (load_status != AurieStatus.Success)
                        {
                            Framework.PrintEx(
                                AurieLogSeverity.Error,
                                $"[ASM] Assembly \"{mod.Path}\" could not be loaded - {load_status.ToString()}!"
                            );

                            continue;
                        }
                        m_LoadedMods.Add(mod);
                        Framework.PrintEx(
                            AurieLogSeverity.Trace,
                            $"[ASM] Loaded file \"{mod}\"."
                            );
                    }
                }

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

            GameVariable my_gamemaker_array = new List<GameVariable> { 5, "string", "hi" };

            return AurieStatus.Success;
        }
    }
}
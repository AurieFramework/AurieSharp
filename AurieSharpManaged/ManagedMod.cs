using AurieSharpInterop;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;
using System.Windows.Markup;
using YYTKInterop;

namespace AurieSharpManaged
{
    internal class ManagedMod
    {
        private AssemblyLoadContext? m_LoadContext;
        private Assembly? m_LoadAssembly;
        private AurieManagedModule m_ManagedModule;

        private string m_Path;
        private bool m_Loaded;
        
        private MethodInfo? LocateMethod(string Name, BindingFlags Flags, Func<MethodInfo, bool> ValidationMethod)
        {
            if (m_LoadAssembly is null)
                return null;

            // Loop all defined types in the assembly
            foreach (var type in m_LoadAssembly.GetTypes())
            {
                // Look for a public, static InitializeMod routine.
                var method = type.GetMethod(Name, Flags);

                // If the InitializeMod routine either:
                // - Does not exist
                // - Does not return AurieStatus (the managed one)
                // - Takes anything but AurieManagedModule
                // it is invalid, and a print should be issued.

                // Just might not be the right class - continue without printing anything.
                if (method == null)
                    continue;

                // If the validation method is okay with this one
                if (ValidationMethod.Invoke(method))
                    return method;
            }

            return null;
        }

        public ManagedMod(string ModPath)
        {
            m_Path = ModPath;
            m_LoadContext = null;
            m_LoadAssembly = null;
            m_Loaded = false;
            m_ManagedModule = new();
            
            if (!File.Exists(m_Path))
                throw new FileNotFoundException("Attempted to create mod with non-existent file", ModPath);
        }
        public AurieStatus Load()
        {
            using var fs = File.Open(m_Path, FileMode.Open, FileAccess.ReadWrite, FileShare.ReadWrite | FileShare.Delete);
            m_LoadContext = new($"ASMContext_{m_Path}", true);
            m_LoadAssembly = m_LoadContext.LoadFromStream(fs);

            MethodInfo? initialize_method = LocateMethod("InitializeMod", BindingFlags.Static | BindingFlags.Public, (PotentialMethod) => 
            {
                // InitializeMod should return AurieStatus.
                if (PotentialMethod.ReturnType != typeof(AurieStatus))
                {
                    Framework.PrintEx(
                        AurieLogSeverity.Warning,
                        $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an InitializeMod routine, but it does not return AurieStatus!"
                    );

                    return false;
                }

                // InitializeMod should take 1 parameter.
                if (PotentialMethod.GetParameters().Length != 1)
                {
                    Framework.PrintEx(
                        AurieLogSeverity.Warning,
                        $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an InitializeMod routine, but it doesn't take 1 argument!"
                    );

                    return false;
                }

                // That one parameter should be a AurieManagedModule type.
                if (PotentialMethod.GetParameters()[0].ParameterType != typeof(AurieManagedModule))
                {
                    Framework.PrintEx(
                        AurieLogSeverity.Warning,
                        $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an InitializeMod routine, but it doesn't take 1 argument!"
                    );

                    return false;
                }

                return true;
            });
            MethodInfo? unload_method = LocateMethod("UnloadMod", BindingFlags.Static | BindingFlags.Public, (PotentialMethod) =>
            {
                // UnloadMod should return void.
                if (PotentialMethod.ReturnType != typeof(void))
                {
                    Framework.PrintEx(
                        AurieLogSeverity.Warning,
                        $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an UnloadMod routine, but it does not return AurieStatus!"
                    );

                    return false;
                }

                // UnloadMod should take 1 parameter.
                if (PotentialMethod.GetParameters().Length != 1)
                {
                    Framework.PrintEx(
                        AurieLogSeverity.Warning,
                        $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an UnloadMod routine, but it doesn't take 1 argument!"
                    );

                    return false;
                }

                // That one parameter should be a AurieManagedModule type.
                if (PotentialMethod.GetParameters()[0].ParameterType != typeof(AurieManagedModule))
                {
                    Framework.PrintEx(
                        AurieLogSeverity.Warning,
                        $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an UnloadMod routine, but it doesn't take 1 argument!"
                    );

                    return false;
                }

                return true;
            });

            if (initialize_method is null)
            {
                Framework.PrintEx(AurieLogSeverity.Trace, "[ASM] initialize_method is null!");
                return AurieStatus.VerificationFailure;
            }

            if (unload_method is null)
            {
                Framework.PrintEx(AurieLogSeverity.Trace, "[ASM] unload_method is null!");
                return AurieStatus.VerificationFailure;
            }

            // Initialize an array because wtf?
            AurieManagedModule[] arguments_array = { m_ManagedModule };

            // Call the ModuleInitialize function.
            // It needs AurieStatus? because it thinks it can be null. It can't.
            var init_result = (AurieStatus?)initialize_method.Invoke(null, arguments_array);
            if (init_result == null)
            {
                throw new NullReferenceException("Managed module init method returned null!");
            }

            m_Loaded = true;
            return init_result.Value;
        }

        public AurieStatus Unload(bool Notify)
        {
            if (m_LoadAssembly is null || m_LoadContext is null)
                return AurieStatus.ObjectNotFound;

            if (Notify)
            {
                // Call UnloadMod if possible.
                MethodInfo? unload_method = LocateMethod("UnloadMod", BindingFlags.Static | BindingFlags.Public, (PotentialMethod) =>
                {
                    // UnloadMod should return void.
                    if (PotentialMethod.ReturnType != typeof(void))
                    {
                        Framework.PrintEx(
                            AurieLogSeverity.Warning,
                            $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an UnloadMod routine, but it does not return AurieStatus!"
                        );

                        return false;
                    }

                    // UnloadMod should take 1 parameter.
                    if (PotentialMethod.GetParameters().Length != 1)
                    {
                        Framework.PrintEx(
                            AurieLogSeverity.Warning,
                            $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an UnloadMod routine, but it doesn't take 1 argument!"
                        );

                        return false;
                    }

                    // That one parameter should be a AurieManagedModule type.
                    if (PotentialMethod.GetParameters()[0].ParameterType != typeof(AurieManagedModule))
                    {
                        Framework.PrintEx(
                            AurieLogSeverity.Warning,
                            $"[ASM] Assembly {m_LoadAssembly.GetName()} contains an UnloadMod routine, but it doesn't take 1 argument!"
                        );

                        return false;
                    }

                    return true;
                });
                if (unload_method is not null)
                    unload_method.Invoke(null, new AurieManagedModule[] { m_ManagedModule });
            }

            Game.Events.RemoveAllScriptHooksForMod(m_ManagedModule);
            Game.Events.RemoveAllBuiltinHooksForMod(m_ManagedModule);

            m_LoadContext.Unload();
            m_Loaded = false;

            return AurieStatus.Success;
        }

        public string Path { get { return m_Path; } }
        public bool Loaded { get { return m_Loaded; } }
    }
}

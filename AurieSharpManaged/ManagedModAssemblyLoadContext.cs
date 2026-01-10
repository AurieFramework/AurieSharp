using AurieSharpInterop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.Loader;
using System.Text;
using System.Threading.Tasks;

namespace AurieSharpManaged
{

    /// <summary>
    /// AssemblyLoadContext implementation with dependency resolution
    /// </summary>
    internal class ManagedModAssemblyLoadContext : AssemblyLoadContext, IDisposable
    {
        private List<Assembly> loadedDependencies;

        public ManagedModAssemblyLoadContext(string name, List<Assembly>? loadedDependencies) : base(name, isCollectible: true)
        {
            this.loadedDependencies = loadedDependencies ?? [];
        }

        public void Dispose()
        {
            loadedDependencies.Clear();
            loadedDependencies = null!;
        }

        protected override Assembly? Load(AssemblyName assemblyName)
        {
            foreach (Assembly assembly in loadedDependencies)
            {
                if (assembly.GetName() == assemblyName)
                {
                    Framework.PrintEx(AurieLogSeverity.Debug, $"[ASM] Resolved dependency \"{assemblyName.FullName}\" for {Name}.");
                    return assembly;
                }
            }
            return base.Load(assemblyName);
        }

    }
}

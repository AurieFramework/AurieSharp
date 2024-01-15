using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Aurie
{
    public enum AurieStatus : Int32
    {
        // The operation completed successfully.
        Success = 0,
        // An invalid architecture was specified.
        InvalidArch,
        // An error occured in an external function call.
        ExternalError,
        // The requested file was not found.
        FileNotFound,
        // The requested access to the object was denied.
        AccessDenied,
        // An object with the same identifier / priority is already registered.
        ObjectAlreadyExists,
        // One or more parameters were invalid.
        InvalidParameter,
        // Insufficient memory is available.
        InsufficientMemory,
        // An invalid signature was detected.
        InvalidSignature,
        // The requested operation is not implemented.
        NotImplemented,
        // An internal error occured in the module.
        ModuleInternalError,
        // The module failed to resolve dependencies.
        ModuleDependencyNotResolved,
        // The module failed to initialize.
        ModuleInitializedFailed,
        // The target file header, directory, or RVA could not be found or is invalid.
        FilePartNotFound,
        // The object was not found.
        ObjectNotFound,
        // The requested resource is unavailable.
        Unavailable
    }

    public class IAurie
    {
        private delegate IntPtr GetFrameworkRoutineDelegate([MarshalAs(UnmanagedType.LPStr)] string RoutineName);
        private GetFrameworkRoutineDelegate m_GetFrameworkRoutine;

        [MethodImpl(MethodImplOptions.NoInlining)]
        private string GetCurrentMethodName()
        {
            var st = new StackTrace();
            var sf = st.GetFrame(1);

            return sf?.GetMethod()?.Name ?? "";
        }

        private delegate AurieStatus ElIsProcessSuspendedDelegate(
            [MarshalAs(UnmanagedType.I1)] ref bool Suspended
        );
        public AurieStatus ElIsProcessSuspended(
            ref bool Suspended
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            return Marshal.GetDelegateForFunctionPointer<ElIsProcessSuspendedDelegate>(function)(ref Suspended);
        }

        private delegate void MmGetFrameworkVersionDelegate(
            ref short Major,
            ref short Minor,
            ref short Patch
        );
        public void MmGetFrameworkVersion(
            ref short Major,
            ref short Minor,
            ref short Patch
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            Marshal.GetDelegateForFunctionPointer<MmGetFrameworkVersionDelegate>(function)(
                ref Major,
                ref Minor,
                ref Patch
            );
        }
        private delegate IntPtr MmSigscanModuleDelegate(
            [MarshalAs(UnmanagedType.LPWStr)] string ModuleName,
            [MarshalAs(UnmanagedType.LPStr)] string Pattern,
            [MarshalAs(UnmanagedType.LPStr)] string PatternMask
        );

        public IntPtr MmSigscanModule(
            string ModuleName,
            string Pattern,
            string PatternMask
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            return Marshal.GetDelegateForFunctionPointer<MmSigscanModuleDelegate>(function)(
                ModuleName,
                Pattern,
                PatternMask
            );
        }

        private delegate IntPtr MmSigscanRegionDelegate(
            UIntPtr RegionBase,
            nuint RegionSize,
            [MarshalAs(UnmanagedType.LPStr)] string Pattern,
            [MarshalAs(UnmanagedType.LPStr)] string PatternMask
        );

        public IntPtr MmSigscanRegion(
            UIntPtr RegionBase,
            nuint RegionSize,
            string Pattern,
            string PatternMask
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            return Marshal.GetDelegateForFunctionPointer<MmSigscanRegionDelegate>(function)(
                RegionBase,
                RegionSize,
                Pattern,
                PatternMask
            );
        }

        // MmCreateHook omitted, no support for std::string_view
        // MmHookExists omitted, no support for std::string_view
        // MmGetHookTrampoline omitted, no support for std::string_view
        // MmRemoveHook omitted, no support for std::string_view
        // Md* omitted

        [return: MarshalAs(UnmanagedType.I1)]
        private delegate bool ObInterfaceExistsDelegate(
            [MarshalAs(UnmanagedType.LPStr)] string InterfaceName
        );

        public bool ObInterfaceExists(
            string InterfaceName
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            return Marshal.GetDelegateForFunctionPointer<ObInterfaceExistsDelegate>(function)(
                InterfaceName
            );
        }

        private delegate AurieStatus ObGetInterfaceDelegate(
            [MarshalAs(UnmanagedType.LPStr)] string InterfaceName,
            ref IntPtr Interface
        );

        public AurieStatus ObGetInterface(
            string InterfaceName,
            ref IntPtr Interface
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            return Marshal.GetDelegateForFunctionPointer<ObGetInterfaceDelegate>(function)(
                InterfaceName,
                ref Interface
            );
        }

        private delegate AurieStatus ObpLookupInterfaceOwnerExportDelegate(
            [MarshalAs(UnmanagedType.LPStr)] string InterfaceName,
            [MarshalAs(UnmanagedType.LPStr)] string ExportName,
            ref IntPtr ExportAddress
        );

        public AurieStatus ObpLookupInterfaceOwnerExport(
            string InterfaceName,
            string ExportName,
            ref IntPtr ExportAddress
        )
        {
            IntPtr function = m_GetFrameworkRoutine(GetCurrentMethodName());
            return Marshal.GetDelegateForFunctionPointer<ObpLookupInterfaceOwnerExportDelegate>(function)(
                InterfaceName,
                ExportName,
                ref ExportAddress
            );
        }

        public IAurie(IntPtr GetFrameworkRoutine)
        {
            m_GetFrameworkRoutine = Marshal.GetDelegateForFunctionPointer<GetFrameworkRoutineDelegate>(
                GetFrameworkRoutine
            );
        }
    }
}

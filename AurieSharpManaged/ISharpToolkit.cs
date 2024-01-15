using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Aurie;

namespace SharpToolkit
{
	public class ISharpToolkit
	{
		private IntPtr m_YYTKInterface;
		private IAurie m_AurieInterface;

		[MethodImpl(MethodImplOptions.NoInlining)]
		private string GetCurrentMethodName()
		{
			var st = new StackTrace();
			var sf = st.GetFrame(1);

			return sf?.GetMethod()?.Name ?? "";
		}

		private delegate void QueryVersionDelegate(
			IntPtr Interface,
			ref short Major,
			ref short Minor,
			ref short Patch
		);

		public void QueryVersion(
			ref short Major,
			ref short Minor,
			ref short Patch
		)
		{
			IntPtr function = IntPtr.Zero;
			
			AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
				"YYTK_Main",
				GetCurrentMethodName(),
				ref function
			);

			if (last_status != AurieStatus.Success)
				throw new Exception("Failed to get YYTK export!");

			Marshal.GetDelegateForFunctionPointer<QueryVersionDelegate>(function)(
				m_YYTKInterface,
				ref Major,
				ref Minor,
				ref Patch
			);
		}

		private delegate AurieStatus GetNamedRoutineIndexDelegate(
			IntPtr Interface,
			[MarshalAs(UnmanagedType.LPStr)] string FunctionName,
			ref int FunctionIndex
		);

		public AurieStatus GetNamedRoutineIndex(
			string FunctionName,
			ref int FunctionIndex
		)
		{
			IntPtr function = IntPtr.Zero;

			AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
				"YYTK_Main",
				GetCurrentMethodName(),
				ref function
			);

			if (last_status != AurieStatus.Success)
				throw new Exception("Failed to get YYTK export!");

			return Marshal.GetDelegateForFunctionPointer<GetNamedRoutineIndexDelegate>(function)(
				m_YYTKInterface,
				FunctionName,
				ref FunctionIndex
			);
		}

		private delegate AurieStatus GetNamedRoutinePointerDelegate(
			IntPtr Interface,
			[MarshalAs(UnmanagedType.LPStr)] string FunctionName,
			ref IntPtr FunctionPointer
		);

		public AurieStatus GetNamedRoutinePointer(
			string FunctionName,
			ref IntPtr FunctionPointer
		)
		{
			IntPtr function = IntPtr.Zero;

			AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
				"YYTK_Main",
				GetCurrentMethodName(),
				ref function
			);

			if (last_status != AurieStatus.Success)
				throw new Exception("Failed to get YYTK export!");

			return Marshal.GetDelegateForFunctionPointer<GetNamedRoutinePointerDelegate>(function)(
				m_YYTKInterface,
				FunctionName,
				ref FunctionPointer
			);
		}

		private delegate void PrintDelegate(
			IntPtr Interface,
			[MarshalAs(UnmanagedType.U1)] byte Color,
			[MarshalAs(UnmanagedType.LPStr)] string Text
		);

		public void Print(
			ConsoleColor Color,
			string Text
		)
		{
			IntPtr function = IntPtr.Zero;

			AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
				"YYTK_Main",
				GetCurrentMethodName(),
				ref function
			);

			if (last_status != AurieStatus.Success)
				throw new Exception("Failed to get YYTK export!");

			Marshal.GetDelegateForFunctionPointer<PrintDelegate>(function)(
				m_YYTKInterface,
				(byte)Color,
				Text
			);
		}

		private delegate void PrintInfoDelegate(
			IntPtr Interface,
			[MarshalAs(UnmanagedType.LPStr)] string Text
		);

		public void PrintInfo(
			string Text
		)
		{
			IntPtr function = IntPtr.Zero;

			AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
				"YYTK_Main",
				GetCurrentMethodName(),
				ref function
			);

			if (last_status != AurieStatus.Success)
				throw new Exception("Failed to get YYTK export!");

			Marshal.GetDelegateForFunctionPointer<PrintInfoDelegate>(function)(
				m_YYTKInterface,
				Text
			);
		}

		private delegate void PrintWarningDelegate(
			IntPtr Interface,
			[MarshalAs(UnmanagedType.LPStr)] string Text
		);

		public void PrintWarning(
			string Text
		)
		{
			IntPtr function = IntPtr.Zero;

			AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
				"YYTK_Main",
				GetCurrentMethodName(),
				ref function
			);

			if (last_status != AurieStatus.Success)
				throw new Exception("Failed to get YYTK export!");

			Marshal.GetDelegateForFunctionPointer<PrintWarningDelegate>(function)(
				m_YYTKInterface,
				Text
			);
		}

        private delegate void PrintErrorDelegate(
			IntPtr Interface,
			[MarshalAs(UnmanagedType.LPStr)] string Filepath,
			int Line,
			[MarshalAs(UnmanagedType.LPStr)] string Text
        );

        public void PrintError(
            string Filepath,
            int Line,
            string Text
        )
        {
            IntPtr function = IntPtr.Zero;

            AurieStatus last_status = m_AurieInterface.ObpLookupInterfaceOwnerExport(
                "YYTK_Main",
                GetCurrentMethodName(),
                ref function
            );

            if (last_status != AurieStatus.Success)
                throw new Exception("Failed to get YYTK export!");

            Marshal.GetDelegateForFunctionPointer<PrintErrorDelegate>(function)(
                m_YYTKInterface,
                Filepath,
				Line,
				Text
            );
        }

        public ISharpToolkit(IAurie AurieInterface)
		{
			m_AurieInterface = AurieInterface;

			AurieInterface.ObGetInterface(
				"YYTK_Main",
				ref m_YYTKInterface
			);

			if (m_YYTKInterface == IntPtr.Zero)
				throw new DllNotFoundException("Native YYToolkit library not loaded!");
		}
	}
}

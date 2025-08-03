using System.Diagnostics;
using System.Runtime.Versioning;
using Aurie.Managed;
using YYTK.Managed;

namespace AurieSharpManaged
{
	[SupportedOSPlatform("windows")]
	public static class AurieSharpManaged
	{
		private static IYYToolkit? m_YYTK = null;
		private static bool load_items = true;
		private static int held_item_id = -1;
		private static string[] ITEM_NAMES = { "lol", "hi" };

		public static void TestCallback(FWCodeEvent Context)
		{
			if (load_items)
			{
				foreach (var item_name in ITEM_NAMES)
				{
					// There is no CallGameScriptEx implemented yet :(
				}
			}
		}

		public static AurieStatus ModuleInitialize()
		{
			IYYToolkit module_interface = new();
            module_interface.CallBuiltin("show_message", new() { new RValue("Hi") });

            return AurieStatus.Success;
		}
	}
}
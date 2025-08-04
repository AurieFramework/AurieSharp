using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using AurieSharpInterop;
using YYTKInterop;

namespace AurieSharpManaged
{
    [SupportedOSPlatform("windows")]
    public static class AurieSharpManaged
    {
        public static void BeforeModifyHealth(ScriptExecutionContext Context)
        {
            double hp_gained = 0;

            if (Context.Arguments[0].TryGetDouble(out hp_gained))
            {
                // If the game is trying to make us lose HP, cancel the call implicitely and return undefined.
                if (hp_gained < 0)
                    Context.OverrideArgument(0, 0);
            }
        }

        [UnmanagedCallersOnly]
        public static AurieStatus ModuleInitialize()
        {
            Game.Events.AddPreScriptNotification("gml_Script_modify_health@Ari@Ari", BeforeModifyHealth);

            return AurieStatus.Success;
        }
    }
}
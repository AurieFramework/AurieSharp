using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using AurieSharpInterop;
using YYTKInterop;

namespace AurieSharpManaged
{
    [SupportedOSPlatform("windows")]
    public static class Demo
    {
        private static bool m_IsFriday = false;
        private static bool m_NotificationSent = false;

        internal static void DisplayFridayNightNotification(ScriptExecutionContext Context)
        {
            Game.Engine.CallScriptEx("gml_Script_create_notification", Context.Self, Context.Other, "misc_local/FridayNightReminder");
        }

        public static void GetMinutesPreCallback(ScriptExecutionContext Context)
        {
            // If we have at least 1 argument, and the first argument is a number
            TimeSpan current_time = TimeSpan.FromSeconds(Context.Arguments[0]);
            Debug.Print($"Current time: {current_time}");

            if (m_IsFriday && current_time.TotalHours >= 20)
            {
                if (!m_NotificationSent)
                {
                    DisplayFridayNightNotification(Context);
                    m_NotificationSent = true;
                }
            }
        }

        public static void GetCalendarDayPostCallback(ScriptExecutionContext Context)
        {
            GameVariable script_result = Context.GetResult();

            int calendar_day;
            if (script_result.TryGetInt32(out calendar_day))
            {
                // The result always has the day that "just ended" - meaning upon Friday start, the result will be Thursday.
                // We account for that by adding 1 to the day.
                DayOfWeek current_day = (DayOfWeek)((calendar_day + 1) % 7);
                m_IsFriday = (current_day == DayOfWeek.Friday);
            }
        }

        [UnmanagedCallersOnly]
        public static AurieStatus ModuleInitialize()
        {
            BeforeScriptCallbackHandler ResetModState = _ =>
            {
                m_IsFriday = false;
                m_NotificationSent = false;
            };

            // On each new day, and on return to the main menu, we reset mod state.
            Game.Events.AddPreScriptNotification("gml_Script_new_day", ResetModState);
            Game.Events.AddPreScriptNotification("gml_Script_setup_main_screen@TitleMenu@TitleMenu", ResetModState);

            // Add the actual used callbacks
            Game.Events.AddPreScriptNotification("gml_Script_get_minutes", GetMinutesPreCallback);
            Game.Events.AddPostScriptNotification("gml_Script_day@Calendar@Calendar", GetCalendarDayPostCallback);

            return AurieStatus.Success;
        }
    }
}
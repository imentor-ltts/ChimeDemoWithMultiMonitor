using ChimeAppLauncherDll.Win32Apis;
using System.Diagnostics;
using System.Net.NetworkInformation;
using System.Runtime.Versioning;
using System.Security.Principal;

namespace ChimeAppLauncherDll
{
    public class ChimeLauncher
    {
        private Process? exeProcess;

        /// <summary>
        /// Helper function to launch the process as System Account(Service)
        /// </summary>
        /// <param name="exePath">The location of the exe</param>
        /// <param name="meetingId">Command line arg to be passed to the Exe</param>
        /// <returns></returns>
        private bool launchAsSystemProcess(string exePath, string cmdArg)
        {
            ADVAPI32.PROCESS_INFORMATION pInfo;
            var exeFileName = $"{exePath} {cmdArg}";
            Logger.LogMessage("exePath: " + exeFileName);
            var result = Win32Interop.CreateInteractiveSystemProcess(
                exeFileName,
                targetSessionId: -1,
                forceConsoleSession: true,
                desktopName: "default",
                hiddenWindow: false,
                out pInfo
            );

            if (!result)
            {
                Logger.LogMessage($"Failed to start the process {exePath} using CreateInteractiveSystemProcess\n");
                return false;
            }
            Logger.LogMessage("Process created using CreateInteractiveSystemProcess");
            string message = $"Process with PId: {pInfo.dwProcessId} and Process Name Demo.exe has started at time {DateTimeOffset.Now}\n";
            exeProcess = Process.GetProcessById(pInfo.dwProcessId);
            Logger.LogMessage(message);
            return true;
        }

        /// <summary>
        /// Helper Function to launch the process as Normal Exe. 
        /// </summary>
        /// <param name="exePath">The location of the exe</param>
        /// <param name="meetingId">Command line arg to be passed to the Exe</param>
        /// <returns></returns>
        private bool launchAsNormalProcess(string exePath, string cmdArg) {
            try
            {
                Logger.LogMessage("Process created using Process.Start");
                ProcessStartInfo info = new ProcessStartInfo();
                info.Arguments = cmdArg;
                info.FileName = exePath;
                info.CreateNoWindow = false;
                info.UseShellExecute = true;
                exeProcess = Process.Start(info);
                if (exeProcess == null)
                {
                    Logger.LogMessage("Failed to start the Process using Process.Start");
                    return false;
                }
                return true;
            }
            catch (Exception ex)
            {
                Logger.LogMessage(ex.Message);
                return false;
            }
        }
        /// <summary>
        /// Launches the Chime Cpp application mentioned as Path
        /// </summary>
        /// <param name="exePath">The Location of the Chime CPP App</param>
        /// <param name="meetingId">The Meeting Id to be used to join the meeting</param>
        /// <returns></returns>
        [SupportedOSPlatform("windows")]
        public bool LaunchChimeApp(string exePath, string meetingId)
        {
            bool success;
            if (WindowsIdentity.GetCurrent().IsSystem)
            {
                success = launchAsSystemProcess(exePath, meetingId);
            }
            else
            {
                success = launchAsNormalProcess(exePath, meetingId);
            }
            if(success)
            {
                Logger.LogMessage($"The  Process has started at {DateTimeOffset.Now}\n");
            }
            return success;
        }


        public Task JoinMeeting(string Url)
        {

        }
    }
}
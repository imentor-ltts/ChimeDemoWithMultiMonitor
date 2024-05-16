using ChimeAppLauncherDll.Win32Apis;
using System.Diagnostics;
using System.Security.Principal;
using Microsoft.Extensions.Configuration;

namespace ChimeAppLauncherDll
{
    public class ChimeLauncher
    {
        private Process? exeProcess;


        /// <summary>
        /// The IsUserLoggedIn method queries all running processes using Process.GetProcesses().
        /// It then checks whether any of the processes have a session ID other than 0 (Session 0 is typically reserved for system processes) and are not the "Idle" process (which runs in Session 0 and represents system idle time).
        /// If such a process is found, it means a user is logged in, so the method returns true; otherwise, it returns false.
        /// </summary>
        /// <returns>True if any User has logged in, else False</returns>
        private static bool IsUserLoggedIn()
        {
            return Process.GetProcesses()
                          .Any(p => p.SessionId != 0 && p.ProcessName != "Idle");
        }
        public bool LaunchChimeApp(string exePath, string meetingId)
        {

            ADVAPI32.PROCESS_INFORMATION pInfo;
            //var exe = @"C:\Chime-Cpp\ars-remote-control\demo\Release\demo.exe";
            if (exeProcess == null)
            {
                //    //if (IsUserLoggedIn())
                //    //{
                //    //    Logger.LogMessage("Process created using Process.Start");
                //    //    _logger.LogInformation("Process created using Process.Start");
                //    //    exeProcess = Process.Start(EXEPATH + $"{id}");
                //    //    if (exeProcess == null)
                //    //    {
                //    //        Logger.LogMessage("Failed to start the Process using Process.Start");
                //    //    }
                //    //    else { 
                //    //        Thread.Sleep(5000);
                //    //        Logger.LogMessage("Demo.exe has started");
                //    //    }

                //    //    return Task.CompletedTask;
                //    //}
                if (WindowsIdentity.GetCurrent().IsSystem /*&& !IsUserLoggedIn()*/)
                {
                    var exeFileName = $"{exePath} {meetingId}";
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
                        Logger.LogMessage("Failed to start the process demo.exe\n");
                        return false;
                    }
                    Logger.LogMessage("Process created using CreateInteractiveSystemProcess");
                    string message = $"Process with PId: {pInfo.dwProcessId} and Process Name Demo.exe has started at time {DateTimeOffset.Now}\n";
                    exeProcess = Process.GetProcessById(pInfo.dwProcessId);
                    Logger.LogMessage(message);
                }
                else
                {
                    try
                    {
                        Logger.LogMessage("Process created using Process.Start");
                        ProcessStartInfo info = new ProcessStartInfo();
                        info.Arguments = meetingId;
                        info.FileName = exePath;
                        info.CreateNoWindow = false;
                        info.UseShellExecute = true;
                        exeProcess = Process.Start(info);
                        if (exeProcess == null)
                        {
                            Logger.LogMessage("Failed to start the Process using Process.Start");
                            return false;
                        }
                    }
                    catch (Exception ex)
                    {
                        Logger.LogMessage(ex.Message);
                    }
                }

            }
            Logger.LogMessage($"The  Process has started at {DateTimeOffset.Now}\n");
            return true;
        }

        /// <summary>
        /// Function to check of the process is running or not.
        /// </summary>
        /// <param name="processName">The Name of the Process to check</param>
        /// <returns>True if the Process is running, else false</returns>
        private static bool IsRunningState(string processName)
        {
            // Get all processes with the specified name
            Process[] processes = Process.GetProcessesByName(processName);

            // If any processes are found, return true; otherwise, return false
            return processes.Length > 0;
        }
        public bool StopChimeApp()
        {
            string message = $"App is stopping at {DateTimeOffset.Now}\n";
            Logger.LogMessage(message);
            if ((exeProcess != null) && (IsRunningState(exeProcess.ProcessName)))
            {
                try
                {
                    exeProcess.Kill();
                    message = $"As the service is stopping, the Demo.exe will stop";
                    Logger.LogMessage(message);
                }
                catch (Exception ex)
                {
                    Logger.LogMessage(ex.Message);
                    return false;
                }
            }
            return true;
        }
    }
}
using ChimeAppLauncherWinService;
using ChimeAppLauncherWinService.Win32Apis;
using Microsoft.Extensions.Options;
using System.Diagnostics;
using System.Security.Principal;

namespace SampleWinService;

internal class Worker : BackgroundService
{
    private readonly ILogger<Worker> _logger;
    private readonly IConfiguration _configuration;
    private readonly CommandLineArgs _commandLineArgs;
    private Process? exeProcess;
    private const string DEFAULTMEETINGID = "801445";
    
    public Worker(ILogger<Worker> logger, IConfiguration configuration, IOptions<CommandLineArgs> commandLineArgs)
    {
        _logger = logger;
        _configuration = configuration;
        _commandLineArgs = commandLineArgs.Value;
        if(_commandLineArgs != null && !string.IsNullOrEmpty(_commandLineArgs.MeetingId ))
        {
            _logger.LogInformation("Command Line args are set to this service");   
        }
        var settings = _configuration.GetSection("LaunchSettings").Get<LaunchSettings>();
        if(settings == null)
        {
            _logger.LogError("Cannot read the Config file appsettigs.json\nUsing Default FileName and Folder Path");
            return;
        }
        Logger.FileName = settings.LogFilePath;
        Logger.LogFolderPath = settings.LogFolderPath;

    }


    public override Task StartAsync(CancellationToken cancellationToken)
    {
        string EXEPATH = @"C:\ProgramData\demo.exe";
        _logger.LogInformation("The  Process has started at {time}\n", DateTimeOffset.Now);

        _logger.LogInformation("Worker running at: {time}\n", DateTimeOffset.Now);

        ADVAPI32.PROCESS_INFORMATION pInfo;
        //var exe = @"C:\Chime-Cpp\ars-remote-control\demo\Release\demo.exe";

        if (exeProcess == null)
        {
            string id = _commandLineArgs.MeetingId;
            var settings = _configuration.GetSection("LaunchSettings").Get<LaunchSettings>();
            if (settings != null)
            {
                EXEPATH = settings.ExePath ?? EXEPATH;
                Logger.LogMessage("Exe Path set using appsettings File: " + EXEPATH + "\n");
            }
            else
            {
                Logger.LogMessage("Exe Path set on Default location: " + EXEPATH + "\n");
            }
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

                var result = Win32Interop.CreateInteractiveSystemProcess(
                    EXEPATH + $" {id}",
                    targetSessionId: -1,
                    forceConsoleSession: true,
                    desktopName: "default",
                    hiddenWindow: false,
                    out pInfo
                );
                if (!result)
                {
                    Logger.LogMessage("Failed to start the process demo.exe\n");
                    _logger.LogInformation("Failed to start the process demo.exe!!!!");
                    return Task.CompletedTask;
                }
                Logger.LogMessage("Process created using CreateInteractiveSystemProcess");
                _logger.LogInformation("Process created using CreateInteractiveSystemProcess");
                string message = $"Process with PId: {pInfo.dwProcessId} and Process Name Demo.exe has started at time {DateTimeOffset.Now}\n";
                exeProcess = Process.GetProcessById(pInfo.dwProcessId);
                Logger.LogMessage(message);
                _logger.LogInformation(message);
            }
            else
            {
                try
                {
                    Logger.LogMessage("Process created using Process.Start");
                    _logger.LogInformation("Process created using Process.Start");
                    exeProcess = Process.Start(EXEPATH + $"{id}");
                    if (exeProcess == null)
                    {
                        _logger.LogError("Failed to start the Process using Process.Start");
                        Logger.LogMessage("Failed to start the Process using Process.Start");

                    }
                }catch(Exception ex )
                {
                    _logger.LogError(ex.Message);
                    Logger.LogMessage(ex.Message);
                }
            }

        }
            return base.StartAsync(cancellationToken);
    }
    /*
     * The IsUserLoggedIn method queries all running processes using Process.GetProcesses().
     * It then checks whether any of the processes have a session ID other than 0 (Session 0 is typically reserved for system processes) and are not the "Idle" process (which runs in Session 0 and represents system idle time).
     * If such a process is found, it means a user is logged in, so the method returns true; otherwise, it returns false.
     */
    private static bool IsUserLoggedIn()
    {
        return Process.GetProcesses()
                      .Any(p => p.SessionId != 0 && p.ProcessName != "Idle");
    }
    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        _logger.LogInformation("CommandLine args: {0}", _commandLineArgs.MeetingId);
        while (!stoppingToken.IsCancellationRequested)
        {
            await Task.Delay(1000, stoppingToken);
        }
    }

    private static bool IsRunningState(string processName)
    {
        // Get all processes with the specified name
        Process[] processes = Process.GetProcessesByName(processName);

        // If any processes are found, return true; otherwise, return false
        return processes.Length > 0;
    }

    public override Task StopAsync(CancellationToken cancellationToken)
    {
        string message = $"Sevice is stopping at {DateTimeOffset.Now}\n";
        Logger.LogMessage(message);
        _logger.LogInformation(message);
        if (IsRunningState(exeProcess.ProcessName))
        {
            try
            {
                exeProcess.Kill();
                message = $"As the service is stopping, the Demo.exe will stop";
                _logger.LogInformation(message);
            }
            catch (Exception ex)
            {
                Logger.LogMessage(ex.Message);
                _logger.LogError(ex.Message);
            }

        }
        return base.StopAsync(cancellationToken);
    }
}

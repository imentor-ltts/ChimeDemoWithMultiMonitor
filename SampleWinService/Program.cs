using ChimeAppLauncherDll;
using ChimeAppLauncherWinService;
using Microsoft.Extensions.Logging.EventLog;
using SampleWinService;

IHost host = Host.CreateDefaultBuilder(args)
    .ConfigureLogging(options =>
    {
        if (OperatingSystem.IsWindows())
        {
            options.AddFilter<EventLogLoggerProvider>(level => level >= LogLevel.Information);
        }
    })
    .ConfigureAppConfiguration((hostContext, configBuilder) =>
    {
        // Add configuration from appsettings.json
        configBuilder.AddJsonFile("appsettings.json", optional: false, reloadOnChange: true);
    })
    .ConfigureServices((hostContext, services) =>
    {
        services.Configure<CommandLineArgs>(hostContext.Configuration.GetSection("CommandLineArgs"));
        services.AddTransient<ChimeLauncher>();
        services.AddHostedService<WorkerDll>();
        if (OperatingSystem.IsWindows())
        {
            services.Configure<EventLogSettings>(config =>
            {
                if (OperatingSystem.IsWindows())
                {
                    config.LogName = "ChimeAppLauncherWinService";
                    config.SourceName = "Chime App Launcher Service Code";
                }
            });
        }
    })
    .UseWindowsService()
    .Build();

host.Run();


using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace ChimeClientStarterApp
{
    internal class FileLogger : ILogger
    {
        /// <summary>
        /// Defines the logFileName.
        /// </summary>
        private readonly string logFileName;

        /// <summary>
        /// Defines the fileLock.
        /// </summary>
        private readonly object fileLock = new object();

        /// <summary>
        /// Defines the logLevel.
        /// </summary>
        private readonly string logLevel;

        /// <summary>
        /// Initializes a new instance of the <see cref="ARSLogger"/> class.
        /// <param name="fileName">Name of file to use for the logging.</param>
        /// <param name="logLevel">Level of information to log.</param>
        /// </summary>
        public FileLogger(string fileName = null, string logLevel = "Information")
        {
            this.logLevel = logLevel;

            if (fileName == null)
            {
                string logFolderPath = ConfigurationManager.AppSettings["logFolderPath"];
                if (String.IsNullOrEmpty(logFolderPath))
                {
                    logFolderPath = @"C:\ProgramData\ARS\Logs";
                }
                if (!Directory.Exists(logFolderPath))
                {
                    Directory.CreateDirectory(logFolderPath);
                }

                this.logFileName = $"{logFolderPath}\\FileLogger_{DateTime.Today.Date:yyyy-MM-dd}.log";
                
                // Create the directory if it doesn't exist
                
            }
            else
            {
                this.logFileName = fileName;
            }
        }

        /// <summary>
        /// The BeginScope.
        /// </summary>
        /// <typeparam name="TState">.</typeparam>
        /// <param name="state">The state<see cref="TState"/>.</param>
        /// <returns>The <see cref="IDisposable"/>.</returns>
        public IDisposable BeginScope<TState>(TState state)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// The Error.
        /// </summary>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        public void Error(string message, [CallerFilePath] string filePath = "", [CallerMemberName] string methodName = "")
        {
            // Log if any one of the Verbose, Information or Error is configured
            if (this.logLevel.Contains("Verbose", StringComparison.OrdinalIgnoreCase) ||
                this.logLevel.Contains("Information", StringComparison.OrdinalIgnoreCase) ||
                this.logLevel.Contains("Error", StringComparison.OrdinalIgnoreCase))
            {
                this.LogMessage("Error", message, filePath, methodName);
            }
        }

        /// <summary>
        /// The Information.
        /// </summary>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        public void Information(string message, [CallerFilePath] string filePath = "", [CallerMemberName] string methodName = "")
        {
            // Log either Verbose is configured or Information is configured
            if (this.logLevel.Contains("Verbose", StringComparison.OrdinalIgnoreCase) ||
                this.logLevel.Contains("Information", StringComparison.OrdinalIgnoreCase))
            {
                this.LogMessage("Information", message, filePath, methodName);
            }
        }

        /// <summary>
        /// The Warning.
        /// </summary>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        public void Warning(string message, [CallerFilePath] string filePath = "", [CallerMemberName] string methodName = "")
        {
            // Log if any one of the Verbose, Information or Error is configured
            if (this.logLevel.Contains("Verbose", StringComparison.OrdinalIgnoreCase) ||
                this.logLevel.Contains("Information", StringComparison.OrdinalIgnoreCase) ||
                this.logLevel.Contains("Error", StringComparison.OrdinalIgnoreCase))
            {
                this.LogMessage("Warning", message, filePath, methodName);
            }
        }

        /// <summary>
        /// The IsEnabled.
        /// </summary>
        /// <param name="logLevel">The logLevel<see cref="LogLevel"/>.</param>
        /// <returns>The <see cref="bool"/>.</returns>
        public bool IsEnabled(LogLevel logLevel)
        {
            // Enable all levels
            return true;
        }

        /// <summary>
        /// The Log.
        /// </summary>
        /// <typeparam name="TState">.</typeparam>
        /// <param name="logLevel">The logLevel<see cref="LogLevel"/>.</param>
        /// <param name="eventId">The eventId<see cref="EventId"/>.</param>
        /// <param name="state">The state<see cref="TState"/>.</param>
        /// <param name="exception">The exception<see cref="Exception"/>.</param>
        /// <param name="formatter">The formatter<see cref="Func{TState, Exception, string}"/>.</param>
        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception exception, Func<TState, Exception, string> formatter)
        {
            var message = formatter.Invoke(state, exception);

            // This is the frame to look at to determine which application API is trying to log information
            var loggerFrame = 1;
            var frame = new StackTrace().GetFrame(loggerFrame);
            var caller = $"{frame.GetMethod().DeclaringType.Name}.{frame.GetMethod().Name}";
            var callerFile = frame.GetFileName();

            switch (logLevel)
            {
                case LogLevel.Trace:
                case LogLevel.Debug:
                case LogLevel.None:
                    this.Verbose(message, callerFile, caller);
                    break;
                case LogLevel.Information:
                    this.Information(message, callerFile, caller);
                    break;
                case LogLevel.Warning:
                    this.Warning(message, callerFile, caller);
                    break;
                case LogLevel.Error:
                case LogLevel.Critical:
                    this.Error(message, callerFile, caller);
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// The Verbose.
        /// </summary>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        public void Verbose(string message, [CallerFilePath] string filePath = "", [CallerMemberName] string methodName = "")
        {
            // Log only if Verbose is configured
            if (this.logLevel.Contains("Verbose", StringComparison.OrdinalIgnoreCase))
            {
                this.LogMessage("Verbose", message, filePath, methodName);
            }
        }

        /// <summary>
        /// The LogMessage.
        /// </summary>
        /// <param name="prefix">The prefix<see cref="string"/>.</param>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        private void LogMessage(string prefix, string message, string filePath, string methodName)
        {
            try
            {
                // Get the file name from file path
                string fileName = Path.GetFileName(filePath);

                // Lock the file until current log write is complete
                lock (this.fileLock)
                {
                    using (StreamWriter file = File.AppendText(this.logFileName))
                    {
                        file.WriteLine($"{DateTime.Now:yyyy-MM-dd HH:mm:ss:fffffff} {Environment.ProcessId}/{Thread.CurrentThread.ManagedThreadId} {prefix}: {fileName} {methodName}(): {message} {Environment.NewLine}");
                    }
                }
            }
            catch (Exception)
            {
                // TODO: Trace error into event log
            }
        }
    }
}

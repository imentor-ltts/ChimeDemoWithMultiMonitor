using Microsoft.Build.Framework;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ChimeClientDotnetStarterApp
{
    internal class FileLogger
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
                //string logFolderPath = Environment.ExpandEnvironmentVariables(@"C:\ProgramData\ARS\Logs");
                string logFolderPath = ConfigurationManager.AppSettings["logFolderPath"];
                if (String.IsNullOrEmpty(logFolderPath))
                {
                    logFolderPath = @"C:\ProgramData\ARS\Logs";
                }
                if (!Directory.Exists(logFolderPath))
                {
                    // Create the directory if it doesn't exist
                    Directory.CreateDirectory(logFolderPath);
                }
                this.logFileName = $"{logFolderPath}\\FileLogger_{DateTime.Today.Date:yyyy-MM-dd}.log";
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
        public void Error(string message,  string filePath = "",  string methodName = "")
        {
            
            this.LogMessage("Error", message, filePath, methodName);
            
        }

        /// <summary>
        /// The Information.
        /// </summary>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        public void Information(string message,  string filePath = "",  string methodName = "")
        {
           this.LogMessage("Information", message, filePath, methodName); 
        }

        /// <summary>
        /// The Warning.
        /// </summary>
        /// <param name="message">The message<see cref="string"/>.</param>
        /// <param name="filePath">The filePath<see cref="string"/>.</param>
        /// <param name="methodName">The methodName<see cref="string"/>.</param>
        public void Warning(string message,  string filePath = "",  string methodName = "")
        {
            // Log if any one of the Verbose, Information or Error is configured
            
                this.LogMessage("Warning", message, filePath, methodName);
            
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
                        file.WriteLine($"{DateTime.Now:yyyy-MM-dd HH:mm:ss:fffffff} {Process.GetCurrentProcess().Id}/{Thread.CurrentThread.ManagedThreadId} {prefix}: {fileName} {methodName}(): {message} {Environment.NewLine}");
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

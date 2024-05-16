using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Identity.Web.Resource;
using RemoteSessionApi.Data;
using RemoteSessionApi.Services;
using System.Diagnostics;
using System.Reflection.Metadata.Ecma335;

namespace RemoteSessionApi.Controllers
{
    //API should be in the same location of the Cpp Application. 
    [Route("api/[controller]")]
    [ApiController]
    public class RemoteSessionController : ControllerBase
    {
        private readonly ILogger<RemoteSessionController> _logger;
        private Process processInfo = null;
        public RemoteSessionController(ILogger<RemoteSessionController> logger)
        {
            _logger = logger;
        }
        [HttpPost]
        [Route("StartRemoteSession")]
        public async Task StartRemoteSession(SessionDetail details)
        {
            const string exePath = "C:\\Chime-Cpp\\ars-remote-control\\demo\\Release\\demo.exe";
            //processInfo = Process.Start(exePath, details.MeetingId);
            if (ApplicationLoader.StartProcessAndBypassUAC(exePath))
                _logger.LogDebug("Process has started");
            else
                _logger.LogError("Process Failed to start");
            await Task.CompletedTask;
        }

        [HttpGet]
        public string GetRemoteSession() 
        {
            var res = "Sample Content";
            return res;
        }
    }
}

//DeviceAgent.

using Microsoft.AspNetCore.Mvc;
using System.Formats.Asn1;

namespace ChimeMeetingApi.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class MeetingController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        public MeetingController(IConfiguration config)
        {
            _configuration = config;
        }
        [HttpGet]
        [Route("/{meetingId}")]
        public async Task<string> GetMeetingDetails(string meetingId)
        {
            string baseUrl = _configuration.GetSection("ApiUrls").Get<ApiUrl>().BaseUrl;
            string endPoint = $"{baseUrl}{meetingId}&e=technician";
            HttpClient client = new HttpClient();
            var task =  await client.GetAsync(endPoint);
            return await task.Content.ReadAsStringAsync();
        }
    }
}
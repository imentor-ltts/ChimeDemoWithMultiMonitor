#include <iostream>
#include "utils\logger\logger.h"
#include "utils\logger\console_logger.h"
#include "session\meeting_session_configuration.h"
#include "session\meeting_session_credentials.h"
#include "session\meeting_session_urls.h"
#include <cpr\cpr.h>
#include <nlohmann\json.hpp>
#include <memory>
#include "session\meeting_session.h"
#include <cmath> // Include this at the top of your file
#include <cstdint>
#include <cstring>
#include "audiovideo/video/video_source.h"
#include "audiovideo/video/video_frame.h"
#include <chrono>
#include <thread>
#include <chrono>
#include "audiovideo\video\i420_video_frame_buffer.h"
#include <windows.h>
#include <vector>
#include <cstdint>
#include "main.h"
#include "DataMessage.h"
#include "ScreenCapture.h"
using namespace std;

BOOL dpi_result = SetProcessDPIAware();

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//helper to print text
void printContent(std::string text) {
    std::cout << text << std::endl;
}


void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

HBITMAP CaptureScreen(int screen_width, int screen_height, HDC hScreenDC) {
    UINT dpi = GetDpiForSystem();
    if (!hScreenDC) {
        // Handle error, possibly log or throw an exception
        std::cout << "Error in getting screen DC" << std::endl;
        return NULL;
    }

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        // Handle error
        std::cout << "Error in creating memory DC" << std::endl;
        ReleaseDC(NULL, hScreenDC);
        return NULL;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
    if (!hBitmap) {
        // Handle error
        std::cout << "Error in creating bitmap" << std::endl;
        ReleaseDC(NULL, hScreenDC);
        DeleteDC(hMemoryDC);
        return NULL;
    }

    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);
    if (!hOldBitmap) {
        std::cout << "Error in selecting bitmap" << std::endl;
        // Handle error
        DeleteObject(hBitmap);
        ReleaseDC(NULL, hScreenDC);
        DeleteDC(hMemoryDC);
        return NULL;
    }

    if (!BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, 0, 0, SRCCOPY)) {

        // Handle BitBlt failure
        std::cout << "Error in BitBlt" << std::endl;
        DeleteObject(hBitmap);
        ReleaseDC(NULL, hScreenDC);
        DeleteDC(hMemoryDC);
        return NULL;
    }

    SelectObject(hMemoryDC, hOldBitmap); // Restore the original bitmap

    ReleaseDC(NULL, hScreenDC);
    DeleteDC(hMemoryDC);

    return hBitmap;
}
// Function to capture the screen
HBITMAP CaptureScreen() {

    UINT dpi = GetDpiForSystem();
    int screen_width;// = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
    int screen_height;// = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);
    
    GetDesktopResolution(screen_width, screen_height);
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) {
        // Handle error, possibly log or throw an exception
        std::cout<<"Error in getting screen DC"<<std::endl;
        return NULL;
    }

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        // Handle error
        std::cout<<"Error in creating memory DC"<<std::endl;
        ReleaseDC(NULL, hScreenDC);
        return NULL;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
    if (!hBitmap) {
        // Handle error
        std::cout<<"Error in creating bitmap"<<std::endl;
        ReleaseDC(NULL, hScreenDC);
        DeleteDC(hMemoryDC);
        return NULL;
    }

    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);
    if (!hOldBitmap) {
        std::cout<<"Error in selecting bitmap"<<std::endl;
        // Handle error
        DeleteObject(hBitmap);
        ReleaseDC(NULL, hScreenDC);
        DeleteDC(hMemoryDC);
        return NULL;
    }
    //bit-block-transfer
    if (!BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, 0, 0, SRCCOPY)) {

        // Handle BitBlt failure
        std::cout<<"Error in BitBlt"<<std::endl;
        DeleteObject(hBitmap);
        ReleaseDC(NULL, hScreenDC);
        DeleteDC(hMemoryDC);
        return NULL;
    }

    SelectObject(hMemoryDC, hOldBitmap); // Restore the original bitmap

    ReleaseDC(NULL, hScreenDC);
    DeleteDC(hMemoryDC);

    return hBitmap;
}


std::vector<uint8_t> GetBitmapData(HBITMAP hBitmap, BITMAPINFO& bmpInfo) {
    if (!hBitmap) {

        // Handle null bitmap error
        std::cout<<"Error in getting bitmap data"<<std::endl;
        return {};
    }

    HDC hDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hDC);

    BITMAP bmp;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {

        // Handle GetObject failure
        std::cout<<"Error in getting object"<<std::endl;
        ReleaseDC(NULL, hDC);
        DeleteDC(hMemDC);
        return {};
    }

    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
    bmpInfo.bmiHeader.biHeight = -bmp.bmHeight; // Negative to flip the image
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    std::vector<uint8_t> bitmapData(bmp.bmWidth * bmp.bmHeight * 4);

    if (!GetDIBits(hMemDC, hBitmap, 0, bmp.bmHeight, bitmapData.data(), &bmpInfo, DIB_RGB_COLORS)) {

        // Handle GetDIBits failure
        std::cout<<"Error in getting DIBits"<<std::endl;
        
        ReleaseDC(NULL, hDC);
        DeleteDC(hMemDC);
        return {};
    }

    ReleaseDC(NULL, hDC);
    DeleteDC(hMemDC);

    return bitmapData;
}

void ConvertRGBToI420Diagnostic(const std::vector<uint8_t>& rgbData, uint8_t* yPlane, uint8_t* uPlane, uint8_t* vPlane, int width, int height) {
    int sizeY = width * height;
    int sizeU = (width / 2) * (height / 2);
    int sizeV = sizeU;  // U and V planes have the same size in I420 format

    // Check if rgbData has the expected size
    if (rgbData.size() != width * height * 4) {
        std::cout << "Error: rgbData size mismatch. Expected " << width * height * 4 << ", got " << rgbData.size() << std::endl;
        return;
    }

    int yIndex = 0, uIndex = 0, vIndex = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 4; // Multiplied by 4 for RGBA

            // Safety check for buffer overflow
            if (i + 2 >= rgbData.size()) {
                std::cout << "Error: Attempted to access rgbData out of bounds at index " << i + 2 << std::endl;
                return;
            }

            uint8_t R = rgbData[i];
            uint8_t G = rgbData[i + 1];
            uint8_t B = rgbData[i + 2];

            int Y = std::round(0.299 * R + 0.587 * G + 0.114 * B);
            int U = std::round(-0.169 * R - 0.331 * G + 0.500 * B + 128);
            int V = std::round(0.500 * R - 0.419 * G - 0.081 * B + 128);

            Y = std::clamp(Y, 16, 235);
            U = std::clamp(U, 16, 240);
            V = std::clamp(V, 16, 240);

            if (yIndex >= sizeY) {
                std::cout << "Error: yIndex out of bounds. yIndex: " << yIndex << ", sizeY: " << sizeY << std::endl;
                return;
            }
            yPlane[yIndex++] = static_cast<uint8_t>(Y);

            if (x % 2 == 0 && y % 2 == 0) {
                if (uIndex < sizeU - 1 && vIndex < sizeV - 1) {
                    uPlane[uIndex++] = static_cast<uint8_t>(U);
                    vPlane[vIndex++] = static_cast<uint8_t>(V);
                }
                else {
                    //std::cout << "Skipping increment to prevent out-of-bounds access." << std::endl;
                }
            }
        }
    }
}



std::shared_ptr<chime::VideoFrame> BitmapToVideoFrame(HBITMAP hBitmap) {
    BITMAPINFO bmpInfo;
    auto rgbData = GetBitmapData(hBitmap, bmpInfo);

    int width = bmpInfo.bmiHeader.biWidth;
    int height = abs(bmpInfo.bmiHeader.biHeight);
    
    // Allocate memory for Y, U, and V planes
    int sizeY = width * height;
    int sizeU = (width / 2) * (height / 2);
    int sizeV = sizeU;  // U and V planes are the same size in I420 format

    std::unique_ptr<uint8_t[]> yPlane(new uint8_t[sizeY]);
    std::unique_ptr<uint8_t[]> uPlane(new uint8_t[sizeU]);
    std::unique_ptr<uint8_t[]> vPlane(new uint8_t[sizeV]);

    ConvertRGBToI420Diagnostic(rgbData, yPlane.get(), uPlane.get(), vPlane.get(), width, height);

    // Create I420VideoFrameBuffer instance
    auto buffer = std::make_shared<chime::I420VideoFrameBuffer>(
        yPlane.release(), uPlane.release(), vPlane.release(),
        width, width / 2, width / 2,
        sizeY, sizeU, sizeV);

    // Create VideoFrame and set its buffer
    auto frame = std::make_shared<chime::VideoFrame>();
    frame->width = width;
    frame->height = height;
    frame->timestamp_ns = 0;  // Set as needed
    frame->rotation = chime::VideoRotation::kRotation0;  // Set as needed
    frame->buffer = buffer;

    /*std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Width of frame: " << width << std::endl;
    std::cout << "Height of frame: " << height << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;*/
    return frame;
}


class SolidColorFrameBuffer : public chime::VideoFrameBuffer {
public:
    SolidColorFrameBuffer(const std::vector<uint8_t>& i420Data, int width, int height)
        : VideoFrameBuffer() {
        width_ = width;
        height_ = height;
        data_ = i420Data;
    }

    const std::vector<uint8_t>& GetI420Data() const {
        return data_;
    }

private:
    std::vector<uint8_t> data_;
};



class FakeFrameVideoSource : public chime::VideoSource {
public:
    FakeFrameVideoSource() : local_sink_(nullptr), hDC(NULL), width(0), height(0) {
    }

    void AddVideoSink(chime::VideoSink* sink) override {
        local_sink_ = sink;
    }

    void RemoveVideoSink(chime::VideoSink* sink) override {
        if (local_sink_ == sink) {
            local_sink_ = nullptr;
        }
    }

    void StartGeneratingFrames() {
        bool continueGenerating = true;

        while (continueGenerating) {
            if (local_sink_) {
                //staticRedFrame->timestamp_ns = GetCurrentTimeInNanoseconds();

                // Use the static red frame
                   //auto frame = generate_red_frame_i420(640,480);

                local_sink_->OnVideoFrameReceived(BitmapToVideoFrame(CaptureExtendedDesktop()));
                
               // get the bitmap handle to the bitmap screenshot, testing on dual screen. 
                //HWND hWnd = GetDesktopWindow();
                //HBITMAP hBmp = GdiPlusScreenCapture(hWnd);
                //local_sink_->OnVideoFrameReceived(BitmapToVideoFrame(hBmp));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Control frame rate
        }
    }

    void StartGeneratingFramesForExtended() {
        bool continueGenerating = true;

        while (continueGenerating) {
            if (local_sink_) {
                //staticRedFrame->timestamp_ns = GetCurrentTimeInNanoseconds();

                // Use the static red frame
                   //auto frame = generate_red_frame_i420(640,480);
                auto hBmp = CaptureScreen(width, height, hDC);
                auto frame = BitmapToVideoFrame(hBmp);
                local_sink_->OnVideoFrameReceived(frame);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Control frame rate
        }
    }

    void SetValues(int screenWidth, int screenHeight, HDC hDC) {
        this->hDC = hDC;
        this->width = screenWidth;
        this->height = screenHeight;
    }
private:
    chime::VideoSink* local_sink_;
    int width;
    int height;
    HDC hDC;

    std::int64_t GetCurrentTimeInNanoseconds() {
        auto now = std::chrono::high_resolution_clock::now();
        auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
        return now_ns.time_since_epoch().count();
    }
    
    std::shared_ptr<chime::VideoFrame> generate_red_frame_i420(int width, int height) {
        // YUV values for red (approximation)
        const uint8_t Y_red = 76;
        const uint8_t U_red = 84;
        const uint8_t V_red = 255;

        // Calculate sizes and strides
        int sizeY = width * height;
        int sizeU = (width / 2) * (height / 2);
        int sizeV = sizeU;  // U and V planes are the same size in I420 format
        int strideY = width;
        int strideU = width / 2;
        int strideV = strideU;

        // Create and initialize Y, U, and V data arrays
        uint8_t* dataY = new uint8_t[sizeY];
        uint8_t* dataU = new uint8_t[sizeU];
        uint8_t* dataV = new uint8_t[sizeV];
        std::memset(dataY, Y_red, sizeY);
        std::memset(dataU, U_red, sizeU);
        std::memset(dataV, V_red, sizeV);

        // Create an I420VideoFrameBuffer instance
        auto red_buffer = std::make_shared<chime::I420VideoFrameBuffer>(
            dataY, dataU, dataV, strideY, strideU, strideV, sizeY, sizeU, sizeV
        );

        // Create a VideoFrame and set its buffer
        auto red_frame = std::make_shared<chime::VideoFrame>();
        red_frame->width = width;
        red_frame->height = height;
        red_frame->timestamp_ns = 0;  // Set as needed
        red_frame->rotation = chime::VideoRotation::kRotation0;  // Set as needed
        red_frame->buffer = red_buffer;
       
        return red_frame;
    }

};


namespace chime_meeting_info {

    // Define structs to hold your JSON data
    struct MediaPlacement {
        std::string AudioHostUrl;
        std::string AudioFallbackUrl;
        std::string ScreenDataUrl;
        std::string ScreenSharingUrl;
        std::string ScreenViewingUrl;
        std::string SignalingUrl;
        std::string TurnControlUrl;
        std::string EventIngestionUrl;
    };

    struct Meeting {
        std::string MeetingId;
        std::string ExternalMeetingId;
        MediaPlacement MediaPlacement;
        std::string MediaRegion;
    };

    struct Attendee {
        std::string ExternalUserId;
        std::string AttendeeId;
        std::string JoinToken;
    };

    struct ResponseData {
        Attendee attendee;
        Meeting meeting;
    };

    // Define a function to parse JSON to your structs
    void from_json(const nlohmann::json& j, MediaPlacement& p) {
        j.at("AudioHostUrl").get_to(p.AudioHostUrl);
        j.at("AudioFallbackUrl").get_to(p.AudioFallbackUrl);
        j.at("ScreenDataUrl").get_to(p.ScreenDataUrl);
        j.at("ScreenSharingUrl").get_to(p.ScreenSharingUrl);
        j.at("ScreenViewingUrl").get_to(p.ScreenViewingUrl);
        j.at("SignalingUrl").get_to(p.SignalingUrl);
        j.at("TurnControlUrl").get_to(p.TurnControlUrl);
        j.at("EventIngestionUrl").get_to(p.EventIngestionUrl);
    }

    void from_json(const nlohmann::json& j, Meeting& m) {
        j.at("MeetingId").get_to(m.MeetingId);
        j.at("ExternalMeetingId").get_to(m.ExternalMeetingId);
        j.at("MediaPlacement").get_to(m.MediaPlacement);
        j.at("MediaRegion").get_to(m.MediaRegion);
    }

    void from_json(const nlohmann::json& j, Attendee& a) {
        j.at("ExternalUserId").get_to(a.ExternalUserId);
        j.at("AttendeeId").get_to(a.AttendeeId);
        j.at("JoinToken").get_to(a.JoinToken);
    }

    void from_json(const nlohmann::json& j, ResponseData& r) {
        j.at("Attendee").get_to(r.attendee);
        j.at("Meeting").get_to(r.meeting);
    }

}  // namespace chime_meeting_info

using namespace chime_meeting_info;

class MyAudioVideoObserver : public chime::AudioVideoObserver {
public:
    chime::ConsoleLogger logger;

    void OnAudioVideoSessionConnecting(
        chime::MeetingSessionStatus session_status) override {
        logger.Info("Session connecting");
    }

    void OnAudioVideoSessionStarted(
        chime::MeetingSessionStatus session_status) override {
        logger.Info("Session started");
    }

    void OnAudioVideoSessionStopped(
        chime::MeetingSessionStatus session_status) override {
        logger.Info("Session stopped with status: " +
            std::to_string(static_cast<int>(session_status.status_code)));
    }

    void OnRemoteVideoSourcesAvailable(
        std::vector<std::shared_ptr<chime::RemoteVideoSource>> remote_video_sources
    ) {
        


            // Add a VideoSink to each RemoteVideoSource
            // You may want to keep track of the video sinks that were added for removal purposes later
          
    }

// ________________________________________________
 //
 // OnDataMessagesReceived
 //
 // PURPOSE: 
 // Handle the Data Messages received as JSON Data and process each message.
 //
 // RETURN VALUE:
 // NONE
 // ________________________________________________
 //
    virtual void OnDataMessagesReceived(const std::vector<chime::DataMessageReceived>& messages)
    {
        auto& msg = messages.front();
        //If data message is about Video Quality
        if (msg.topic == "VideoQuality") {
            std::cout << "Video Quality changed" << std::endl;
            return;
        }
        //If the data message is Input Message
        else {
            auto json_response = nlohmann::json::parse(msg.data);
            MouseEventMessages::DataMessage dm = json_response.get<MouseEventMessages::DataMessage>();
            INT32 xPos = dm.xPos;
            INT32 yPos = dm.yPos;
            Conversion::GetAbsoluteCoordinates(xPos, yPos);
            
           if (dm.eventId == "paste") {
                KeyBoardService::SendText(dm.message);
            }
            else if (dm.eventId == "run") {
               KeyBoardService::SendKeyDown("Meta");
               KeyBoardService::SendKeyDown("R");
               KeyBoardService::SendKeyUp("R");
               KeyBoardService::SendKeyUp("Meta");
		   }
            else if (dm.eventId == "NewWin") {
               KeyBoardService::SendAltKeyDown();
               KeyBoardService::SendKeyDown("N");
           }
			else if (dm.eventId == "SpKeys1") {
			   KeyBoardService::SendKeyDown("Control");
               KeyBoardService::SendAltKeyDown();
               KeyBoardService::SendKeyDown("S");
			   KeyBoardService::SendKeyDown("K");
			   KeyBoardService::SendKeyUp("K");
			   KeyBoardService::SendKeyUp("S");
               KeyBoardService::SendAltKeyUp();
			   KeyBoardService::SendKeyUp("Control");
           }
            else if (dm.eventId == "AltDown") {
               KeyBoardService::SendAltKeyDown();
           }
            else if (dm.eventId == "AltUp") {
               KeyBoardService::SendAltKeyUp();
           }
            else if (dm.eventId == "showMenus") {
               KeyBoardService::SendKeyDown("Meta");
               KeyBoardService::SendKeyDown("X");
               KeyBoardService::SendKeyUp("X");
               KeyBoardService::SendKeyUp("Meta");
            }
            else if (dm.eventId == "WM_LBUTTONDOWN") {
                MouseService::MouseClick();
            }
            else if (dm.eventId == "WM_LBUTTONUP") {
               MouseService::SendMouseUp();
           }
            else if (dm.eventId == "WM_RBUTTONDOWN") {
                MouseService::SendRButtonDown(xPos, yPos);
            }
            else if (dm.eventId == "WM_MOUSEMOVE") {
                MouseService::SendMouseMove(xPos, yPos);
            }
            else if (dm.eventId == "WM_KEYDOWN") {
                KeyBoardService::SendKeyDown(dm.message); //message will be the key pressed
            }
            else if (dm.eventId == "WM_KEYUP") {
                KeyBoardService::SendKeyUp(dm.message); //message will be the key pressed
            }
            else {
                logger.Info(dm.message);
            }
            return;
        }
    }
};



void joinAsAttendee(std::string name, cMonitorsVec monitorInfo) {

    std::unique_ptr<chime::Logger> logger = std::make_unique<chime::ConsoleLogger>(chime::LogLevel::kDebug);

    // Replace 'your_url' with the actual URL you want to call
    auto response = cpr::Get(cpr::Url{ "https://pulud6u8je.execute-api.us-east-1.amazonaws.com/Prod/join?m=123&e=" + name });

    // Parse the JSON response
    auto json_response = nlohmann::json::parse(response.text);

    // Deserialize JSON into C++ structs
    ResponseData data = json_response.get<ResponseData>();

    // Now you can use 'data' as a C++ object
    std::cout << "Attendee ID: " << data.attendee.AttendeeId << std::endl;
    std::cout << "Meeting ID: " << data.meeting.MeetingId << std::endl;
    std::cout << "Audio Host URL: " << data.meeting.MediaPlacement.AudioHostUrl << std::endl;
    std::cout << "Signaling URL: " << data.meeting.MediaPlacement.SignalingUrl << std::endl;
    std::cout << "External User ID: " << data.attendee.ExternalUserId << std::endl;
    std::cout << "Join Token: " << data.attendee.JoinToken << std::endl;

    chime::MeetingSessionCredentials credentials{ data.attendee.AttendeeId,
                                             data.attendee.ExternalUserId,
                                             data.attendee.JoinToken };

    chime::MeetingSessionURLs urls{ data.meeting.MediaPlacement.AudioHostUrl,
                                   data.meeting.MediaPlacement.SignalingUrl };

    chime::MeetingSessionConfiguration configuration{ data.meeting.MeetingId,
                                                     data.meeting.ExternalMeetingId,
                                                     std::move(credentials),
                                                     std::move(urls) };


    std::shared_ptr<chime::MeetingSessionDependencies> session_dependencies =
        std::make_shared<chime::MeetingSessionDependencies>();
    session_dependencies->logger = std::move(logger);

    std::shared_ptr<chime::MeetingSession> session =
        chime::MeetingSession::Create(configuration, session_dependencies);


    MyAudioVideoObserver my_audio_video_observer = MyAudioVideoObserver();

    session->audio_video->AddAudioVideoObserver(&my_audio_video_observer);


    std::shared_ptr<FakeFrameVideoSource> fake_frame_source = std::make_shared<FakeFrameVideoSource>();
    auto rcMonitor = monitorInfo.rcMonitors.at(1);
    
    //Place to call SetValues...........................
    int monitorWidth = rcMonitor.right - rcMonitor.left;
    int monitorHeight = rcMonitor.bottom - rcMonitor.top;
    std::cout << "----------------------------------------------------\n";
    std::cout << "Monitor Width: " << monitorWidth << "Monitor Height: " << monitorHeight << std::endl;
	std::cout << "----------------------------------------------------\n";

	MONITORINFOEX monitorInfoEx;
    monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
    auto hMon = monitorInfo.hMonitors.at(1);
	GetMonitorInfo(hMon, &monitorInfoEx);
	HDC hScreenDC = CreateDC(L"DISPLAY", monitorInfoEx.szDevice, NULL, NULL);

	//HDC hScreenDC = monitorInfo.hdcMonitors.at(1);
	if (hScreenDC == NULL) {
        std::cout << "Not possible to get the DC of second monitor\n";
        return;
    }

    fake_frame_source->SetValues(monitorWidth, monitorHeight, hScreenDC);

    chime::LocalVideoConfiguration local_video_config;
    local_video_config.modality = chime::Modality::kContent;
    local_video_config.codecPreferences = { chime::VideoCodecCapability::H264_BASELINE_PROFILE };
    session->audio_video->AddLocalVideo(fake_frame_source, local_video_config);

    // Start generating frames in a separate thread
    // Directly call StartGeneratingFrames without creating a new threadl
    session->audio_video->Start();
    std::thread frameGenerationThread2(&FakeFrameVideoSource::StartGeneratingFramesForExtended, fake_frame_source);
    frameGenerationThread2.join();
    //fake_frame_source->StartGeneratingFramesForExtended(); 
}

void joinFirstAttendee(string meetingId) {
    std::unique_ptr<chime::Logger> logger = std::make_unique<chime::ConsoleLogger>(chime::LogLevel::kDebug);

    string baseUrl = "https://pulud6u8je.execute-api.us-east-1.amazonaws.com/Prod/join?m=" + meetingId + "&e=abhi";
    // Replace 'your_url' with the actual URL you want to call
    auto response = cpr::Get(cpr::Url{ baseUrl });

    // Parse the JSON response
    auto json_response = nlohmann::json::parse(response.text);

    // Deserialize JSON into C++ structs
    ResponseData data = json_response.get<ResponseData>();

    // Now you can use 'data' as a C++ object
    std::cout << "Attendee ID: " << data.attendee.AttendeeId << std::endl;
    std::cout << "Meeting ID: " << data.meeting.MeetingId << std::endl;
    std::cout << "Audio Host URL: " << data.meeting.MediaPlacement.AudioHostUrl << std::endl;
    std::cout << "Signaling URL: " << data.meeting.MediaPlacement.SignalingUrl << std::endl;
    std::cout << "External User ID: " << data.attendee.ExternalUserId << std::endl;
    std::cout << "Join Token: " << data.attendee.JoinToken << std::endl;



    chime::MeetingSessionCredentials credentials{ data.attendee.AttendeeId,
                                             data.attendee.ExternalUserId,
                                             data.attendee.JoinToken };

    chime::MeetingSessionURLs urls{ data.meeting.MediaPlacement.AudioHostUrl,
                                   data.meeting.MediaPlacement.SignalingUrl };

    chime::MeetingSessionConfiguration configuration{ data.meeting.MeetingId,
                                                     data.meeting.ExternalMeetingId,
                                                     std::move(credentials),
                                                     std::move(urls) };


    std::shared_ptr<chime::MeetingSessionDependencies> session_dependencies =
        std::make_shared<chime::MeetingSessionDependencies>();
    session_dependencies->logger = std::move(logger);

    std::shared_ptr<chime::MeetingSession> session =
        chime::MeetingSession::Create(configuration, session_dependencies);

    MyAudioVideoObserver my_audio_video_observer = MyAudioVideoObserver();

    session->audio_video->AddAudioVideoObserver(&my_audio_video_observer);


    std::shared_ptr<FakeFrameVideoSource> fake_frame_source = std::make_shared<FakeFrameVideoSource>();

    chime::LocalVideoConfiguration local_video_config;
    local_video_config.modality = chime::Modality::kContent;
    local_video_config.codecPreferences = { chime::VideoCodecCapability::H264_BASELINE_PROFILE };
    session->audio_video->AddLocalVideo(fake_frame_source, local_video_config);
    // Start generating frames in a separate thread
    // Directly call StartGeneratingFrames without creating a new threadl
    session->audio_video->Start();
    std::thread frameGenerationThread(&FakeFrameVideoSource::StartGeneratingFrames, fake_frame_source);
    frameGenerationThread.join();
    //fake_frame_source->StartGeneratingFrames();
   
    //session->audio_video->Stop();
}



int main(int argc, char* argv[])
{
    cMonitorsVec mons;
	std::unique_ptr<chime::Logger> logger = std::make_unique<chime::ConsoleLogger>(chime::LogLevel::kDebug);

    int noOfMonitors = mons.iMonitors.size();
   /* if (noOfMonitors > 1) {
        joinAsAttendee("Phani", mons);
    }*/
    if (argv[1] == NULL) {
        joinFirstAttendee("123");
    }
    else {
        std::string meetingId = argv[1];
        cout << meetingId << endl;
        joinFirstAttendee(meetingId);
    }
    std::cout << "Press any key to end the meeting" << std::endl;

    std::cin.get();

}
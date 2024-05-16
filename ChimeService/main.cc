#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "utils\logger\logger.h"
#include "utils\logger\console_logger.h"
#include "session\meeting_session_configuration.h"
#include "session\meeting_session_credentials.h"
#include "session\meeting_session_urls.h"
#include <cpr\cpr.h>
#include <nlohmann\json.hpp>
#include <string>
#include <memory>
#include "session\meeting_session.h"
#include <cmath> // Include this at the top of your file
#include <cstdint>
#include <cstring>
#include "audiovideo/video/video_source.h"
#include "audiovideo/video/video_frame.h"
#include <memory>
#include <chrono>
#include <thread>
#include <chrono>
#include "audiovideo\video\i420_video_frame_buffer.h"
#include <memory>
#include <cstring>
#include <windows.h>
#include <vector>
#include <cstdint>
#include "main.h"
#include"DataMessage.h"
#include"ScreenCapture.h"
#include "EventConstants.h"

using namespace std;

BOOL dpi_result = SetProcessDPIAware();
std::shared_ptr<chime::MeetingSession> session = NULL;

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

//helper to print text
void printContent(std::string text) {
	std::cout << text << std::endl;
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

std::shared_ptr<chime::VideoFrame> GetVideoFrame() {

	HBITMAP hBitmap = CaptureScreen();
	BITMAPINFO bmpInfo;
	auto rgbData = GetBitmapData(hBitmap, bmpInfo);
	DeleteObject(hBitmap); // Delete HBITMAP to avoid memory leak

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

	/* std::cout << "--------------------------------------------------" << std::endl;
	 std::cout << "Width of frame: " << width << std::endl;
	 std::cout << "Height of frame: " << height << std::endl;
	 std::cout << "--------------------------------------------------" << std::endl;*/
	return frame;
}

class FrameCapturer {
public:
	FrameCapturer() : width_(0), height_(0), sizeY_(0), sizeU_(0), sizeV_(0) {}

	std::shared_ptr<chime::VideoFrame> GetVideoFrame() {
		//logToFile("Started to generate Video Frame");
		HBITMAP hBitmap = CaptureScreen();
		BITMAPINFO bmpInfo;
		auto rgbData = GetBitmapData(hBitmap, bmpInfo);
		DeleteObject(hBitmap); // Delete HBITMAP to avoid memory leak

		int width = bmpInfo.bmiHeader.biWidth;
		int height = abs(bmpInfo.bmiHeader.biHeight);

		// Check if buffer sizes need to be updated
		UpdateBufferSizes(width, height);

		ConvertRGBToI420Diagnostic(rgbData, yPlane_.get(), uPlane_.get(), vPlane_.get(), width, height);

		auto buffer = std::make_shared<chime::I420VideoFrameBuffer>(
			yPlane_.get(), uPlane_.get(), vPlane_.get(),
			width, width / 2, width / 2,
			sizeY_, sizeU_, sizeV_);

		auto frame = std::make_shared<chime::VideoFrame>();
		frame->width = width;
		frame->height = height;
		frame->timestamp_ns = 0;
		frame->rotation = chime::VideoRotation::kRotation0;
		frame->buffer = buffer;
		//logToFile("Ended Generating Video Frame and returning it");
		return frame;
	}

private:
	void UpdateBufferSizes(int width, int height) {
		int newSizeY = width * height;
		int newSizeU = (width / 2) * (height / 2);
		int newSizeV = newSizeU;

		if (newSizeY != sizeY_ || newSizeU != sizeU_ || newSizeV != sizeV_) {
			yPlane_.reset(new uint8_t[newSizeY]);
			uPlane_.reset(new uint8_t[newSizeU]);
			vPlane_.reset(new uint8_t[newSizeV]);

			sizeY_ = newSizeY;
			sizeU_ = newSizeU;
			sizeV_ = newSizeV;
		}
	}

	int width_, height_;
	int sizeY_, sizeU_, sizeV_;
	std::unique_ptr<uint8_t[]> yPlane_;
	std::unique_ptr<uint8_t[]> uPlane_;
	std::unique_ptr<uint8_t[]> vPlane_;
};

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
	FakeFrameVideoSource() : local_sink_(nullptr) {
	}

	void AddVideoSink(chime::VideoSink* sink) override {
		local_sink_ = sink;
	}

	void RemoveVideoSink(chime::VideoSink* sink) override {
		if (local_sink_ == sink) {
			local_sink_ = nullptr;
		}
	}

	void StartGeneratingFrames	() {
		bool continueGenerating = true;
		FrameCapturer capturer;



		while (continueGenerating) {
			if (local_sink_) {
				//staticRedFrame->timestamp_ns = GetCurrentTimeInNanoseconds();

				// Use the static red frame
				   //auto frame = generate_red_frame_i420(640,480);
				//GetVideoFrame();

				 // Capture a frame
				auto frame = capturer.GetVideoFrame();
				local_sink_->OnVideoFrameReceived(frame);
				//logToFile("Frames are sent to the Vidoe Sink");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Control frame rate
		}
	}

private:
	chime::VideoSink* local_sink_;

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
	virtual void OnAttendeeJoined(const Attendee& attendee) {
		logger.Info("Attendee with Id " + attendee.ExternalUserId + " has joined");
	}

	virtual void OnAttendeeLeft(const Attendee& attendee) {
		logger.Info("Attendee with ID " + attendee.ExternalUserId + "has left the meeting");
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
		logToFile("Data Message is recieved: " + msg.data);
		//If data message is about Video Quality
		if (msg.topic == "VideoQuality") {
			std::cout << "Video Quality changed" << std::endl;
			logToFile("Video Quality changed");
			return;
		}
		//If the data message is Input Message
		else {
			auto json_response = nlohmann::json::parse(msg.data);
			MouseEventMessages::DataMessage dm = json_response.get<MouseEventMessages::DataMessage>();
			INT32 xPos = dm.xPos;
			INT32 yPos = dm.yPos;
			UINT retVal = 0;
			char strText[100];

			Conversion::GetAbsoluteCoordinates(xPos, yPos);
			switch (dm.eventId)
			{
			case CM_PASTE:
				KeyBoardService::SendText(dm.message);
				logToFile("Paste operation is done");
				break;
			case CM_RUN:
				KeyBoardService::SendKeyDown("Meta");
				KeyBoardService::SendKeyDown("R");
				KeyBoardService::SendKeyUp("R");
				KeyBoardService::SendKeyUp("Meta");
				logToFile("Run command is executed");
				break;
			case CM_NEWWIN:
				KeyBoardService::SendAltKeyDown();
				KeyBoardService::SendKeyDown("N");
				logToFile("New Window Command is executed");
				break;
			case CM_SPKEYS1:
				KeyBoardService::SendKeyDown("Control");
				KeyBoardService::SendAltKeyDown();
				KeyBoardService::SendKeyDown("S");
				KeyBoardService::SendKeyDown("K");
				KeyBoardService::SendKeyUp("K");
				KeyBoardService::SendKeyUp("S");
				KeyBoardService::SendAltKeyUp();
				KeyBoardService::SendKeyUp("Control");
				logToFile("Sp Key command Command is executed");
				break;
			case CM_ALTDOWN:
				KeyBoardService::SendAltKeyDown();
				break;
			case CM_ALTUP:
				KeyBoardService::SendAltKeyUp();
				logToFile("Alt key command is executed");
				break;
			case CM_SHOWMENUS:
				KeyBoardService::SendKeyDown("Meta");
				KeyBoardService::SendKeyDown("X");
				KeyBoardService::SendKeyUp("X");
				KeyBoardService::SendKeyUp("Meta");
				logToFile("Show Menus command is executed");
				break;
			case CM_LBUTTONDOWN:
				retVal = MouseService::SendLButtonDown(xPos, yPos);
				sprintf(strText, "The Left Mouse button retVal is : %d", retVal);
				logToFile(strText);
				break;
			case CM_LBUTTONUP:
				retVal = MouseService::SendMouseUp();
				logToFile("Mouse Left Button Up command is executed");
				break;
			case CM_RBUTTONDOWN:
				retVal = MouseService::SendRButtonDown(xPos, yPos);
				retVal = MouseService::SendRButtonUp(xPos, yPos);
				logToFile("Mouse Right Button down command is executed");
				break;
			case CM_RBUTTONUP:
				retVal = MouseService::SendRButtonUp(xPos, yPos);
				logToFile("Mouse Right Button down command is executed");
				break;
			case CM_MOUSEMOVE:
				retVal = MouseService::SendMouseMove(xPos, yPos);
				logToFile("Mouse Move command is executed");
				break;
			case CM_KEYDOWN:
				retVal = KeyBoardService::SendKeyDown(dm.message); //message will be the key pressed
				sprintf(strText, "RetVal for Send Key Down: %d", retVal);
				logToFile(strText);
				break;
			case CM_KEYUP:
				KeyBoardService::SendKeyUp(dm.message); //message will be the key pressed
				break;
			case CM_CLOSESESSION:
				if (session != NULL) {
					session->audio_video->Stop();
					StopService("RemoteLauncherService3");
					logger.Info("Session is being closed now. Other joinees should not be able to view the Remote Desktop");
					logToFile("Session is being closed now. Other joinees should not be able to view the Remote Desktop");
					HANDLE handle = GetCurrentProcess();
					TerminateProcess(handle, 0);
					logToFile("Application is closing now!!!");
				}
				break;
			default:
				logger.Info(dm.message);
				break;
			}
		}
	}
};

//Create an Web API that has an end point that calls the Meeting Lambda  API and that will be triggered from the CPP App.
 

std::shared_ptr<chime::MeetingSession> createSession(string meetingId) {
	std::unique_ptr<chime::Logger> logger = std::make_unique<chime::ConsoleLogger>(chime::LogLevel::kDebug);

	string baseUrl = "https://pulud6u8je.execute-api.us-east-1.amazonaws.com/Prod/join?m=" + meetingId + "&e=abhi";
	//string baseUrl = "https://localhost:7188/" + meetingId;

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
	return session;
}


void joinAsFirstAttendee(string meetingId) {
	session = createSession(meetingId);
	logToFile("Meeting Session has been created");
	MyAudioVideoObserver my_audio_video_observer = MyAudioVideoObserver();

	session->audio_video->AddAudioVideoObserver(&my_audio_video_observer);

	std::shared_ptr<FakeFrameVideoSource> fake_frame_source = std::make_shared<FakeFrameVideoSource>();

	chime::LocalVideoConfiguration local_video_config;
	local_video_config.modality = chime::Modality::kContent;
	local_video_config.codecPreferences = { chime::VideoCodecCapability::H264_MAIN_PROFILE };
	session->audio_video->AddLocalVideo(fake_frame_source, local_video_config);

	// Start generating frames in a separate thread
	// Directly call StartGeneratingFrames without creating a new threadl
	std::thread frameGenerationThread(&FakeFrameVideoSource::StartGeneratingFrames, fake_frame_source);
	session->audio_video->Start();
	//logToFile("Frames have started to be sent");
	fake_frame_source->StartGeneratingFrames();
	//logToFile("Frames are getting generated");
	std::cout << "Press any key to end the meeting" << std::endl;

	std::cin.get();
}

void SimulateInputActions() {
	/*
	* Sending a dblclick
	* Sending the password
	* Sending the
	*/
	Sleep(10000);//10 secs when I will lock the screen 
	MouseService::MouseClick();
	Sleep(2000);
	KeyBoardService::SendKeyDown("1");
	KeyBoardService::SendKeyDown("9");
	KeyBoardService::SendKeyDown("7");
	KeyBoardService::SendKeyDown("6");
	MouseService::MouseClick();
}
int main(int argc, char* argv[])
{
	//SimulateInputActions();
	//joinAsFirstAttendee("801445");
	
	if (argv[1] == NULL) {
		joinAsFirstAttendee("801445");
	}
	else {
		std::string meetingId = argv[1];
		cout << meetingId << endl;
		joinAsFirstAttendee(meetingId);
	}
	std::cout << "Press any key to end the meeting" << std::endl;
	std::cin.get();
}

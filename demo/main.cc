// --------------------------------------------------------------------------------------------------------------------
// <copyright file="main.cc" company="Siemens Ultrasound">
// Copyright(c) Since 2022 by Siemens Healthineers
// All Rights Reserved.
// No part of this software may be reproduced or transmitted in any
// form or by any means including photocopying or recording without
// written permission of the copyright owner.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
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
#include "FileLogger.h"
#include "EventConstants.h"
#include "EventMessage.h"
#include "KeyBoardMouseService.h"
#include "BitmapFileCreator.h"
#include "main.h"
#include"DirectXDemo.h"
#include "SAS.h"


using namespace std;
//Need to call this function to make the Process Dpi Aware
BOOL dpi_result = SetProcessDPIAware();


std::shared_ptr<chime::MeetingSession> session = NULL;
std::shared_ptr<chime::MeetingSession> session2 = NULL;



//index value to be stored so that it can select the monitor. 
static int monitorIndex = 0;

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
// ________________________________________________
//
// GetVideoFrame
//
// PURPOSE: 
// Helper Function to Get the Video Frames
//
// RETURN VALUE:
// shared_ptr of VideoFrame: The pointer to the VideoFrames that are generated. 
// ________________________________________________
//
std::shared_ptr<chime::VideoFrame> GetVideoFrame() {
	HBITMAP hBitmap = CaptureScreen(monitorIndex);
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
	return frame;
}

/// <summary>
/// Class that contains the Frame Capturer
/// </summary>
class FrameCapturer {
public:
	FrameCapturer() : width_(0), height_(0), sizeY_(0), sizeU_(0), sizeV_(0) {}

	/// <summary>
	/// Helper function to create ScreenShot and save it in a common folder. 
	/// </summary>
	/// <param name="hBitmap">The Bitmap image to capture</param>
	void createScreenShot(HBITMAP hBitmap) {
		auto fileName = GenerateTimestampFilename();
		auto res = SaveHBitmapToFile(hBitmap, fileName.c_str());
		if (!res) {
			logger.Error("Failed to create the Image");
			auto err = Conversion::GetLastErrorAsString(GetLastError());
			logger.Error(err);
			FileLogger::LogToFile(err);
		}
	}
	std::shared_ptr<chime::VideoFrame> GetVideoFrame(int index) {
		//logToFile("Started to generate Video Frame");
		HBITMAP hBitmap = CaptureScreen(index);//1 for secondary and 0 for primary monitor. 
		if (hBitmap == NULL) {
			FileLogger::LogToFile("Failed to get the Bitmap Image from the Capture Screen Method");
			return NULL;
		}
		
		//createScreenShot(hBitmap);
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

		FileLogger::LogToFile("--------------------------------------------------\n");
		char strText[100];
		sprintf(strText, "Width and Height of frame: %d X %d", width, height);
		FileLogger::LogToFile(strText);
		logger.Info(strText);
		FileLogger::LogToFile("--------------------------------------------------\n");
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

	void StartGeneratingFrames	(int index) {
		bool continueGenerating = true;
		FrameCapturer capturer;


		
		while (continueGenerating) {
			if (local_sink_) {
				 // Capture a frame
				auto frame = capturer.GetVideoFrame(index);
				
				local_sink_->OnVideoFrameReceived(frame);
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
/// <summary>
/// Class that represents the Observer for handling messages from the Chime meetings
/// </summary>
class MyAudioVideoObserver : public chime::AudioVideoObserver {
public:
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
		logger.Info("Session started with status code " + std::to_string(static_cast<int>(session_status.status_code)));
	}

	void OnAudioVideoSessionStopped(
		chime::MeetingSessionStatus session_status) override {
		auto msg = "Session stopped with status: " +
			std::to_string(static_cast<int>(session_status.status_code));
		logger.Info(msg);
		if (static_cast<int>(session_status.status_code) != 0) {
			logger.Error("Session terminated abruptly");
			FileLogger::LogToFile(msg);
		}
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
		FileLogger::LogToFile("The Monitor Coordinates" + to_string(static_cast<int>(selectedMonitorRect.left)));
		auto& msg = messages.front();
		FileLogger::LogToFile("Data Message is received: " + msg.data);
		//If data message is about Video Quality
		if (msg.topic == "VideoQuality") {
			std::cout << "Video Quality changed" << std::endl;
			FileLogger::LogToFile("Video Quality changed");
			return;
		}
		//If the data message is Input Message
		else {
			auto json_response = nlohmann::json::parse(msg.data);
			EventMessages::DataMessage dm = json_response.get<EventMessages::DataMessage>();
			INT32 xPos = dm.xPos;
			INT32 yPos = dm.yPos;
			UINT retVal = 0;
			char strText[100];
			Conversion::GetAbsoluteCoordinates(xPos, yPos);
			switch (dm.eventId)
			{
			case CM_PASTE:
				service.SendText(dm.message);
				FileLogger::LogToFile("Paste operation is done");
				break;
			case CM_RUN:
				service.SendKeyDown("Meta");
				service.SendKeyDown("R");
				service.SendKeyUp("R");
				service.SendKeyUp("Meta");
				FileLogger::LogToFile("Run command is executed");
				break;
			case CM_NEWWIN:
				service.SendAltKeyDown();
				service.SendKeyDown("N");
				FileLogger::LogToFile("New Window Command is executed");
				break;
			case CM_SPKEYS1:
				service.SendKeyDown("Control");
				service.SendAltKeyDown();
				service.SendKeyDown("S");
				service.SendKeyDown("K");
				service.SendKeyUp("K");
				service.SendKeyUp("S");
				service.SendAltKeyUp();
				service.SendKeyUp("Control");
				FileLogger::LogToFile("Sp Key command Command is executed");
				break;
			case CM_ALTDOWN:
				service.SendAltKeyDown();
				break;
			case CM_ALTUP:
				service.SendAltKeyUp();
				FileLogger::LogToFile("Alt key command is executed");
				break;
			case CM_SHOWMENUS:
				service.SendKeyDown("Meta");
				service.SendKeyDown("X");
				service.SendKeyUp("X");
				service.SendKeyUp("Meta");
				FileLogger::LogToFile("Show Menus command is executed");
				break;
			case CM_LBUTTONDOWN:
				sprintf(strText, "The Actual MouseX: %f", dm.xPos);
				logger.Info(strText);
				sprintf(strText, "The Actual MouseY: %f", dm.yPos);
				logger.Info(strText);
				retVal = service.SendLButtonDown(xPos, yPos);
				sprintf(strText, "The Left Mouse button retVal is : %d", retVal);
				FileLogger::LogToFile(strText);
				break;
			case CM_LBUTTONUP:
				retVal = service.SendMouseUp(xPos, yPos);
				FileLogger::LogToFile("Mouse Left Button Up command is executed");
				break;
			case CM_RBUTTONDOWN:
				retVal = service.SendRButtonDown(xPos, yPos);
				retVal = service.SendRButtonUp(xPos, yPos);
				FileLogger::LogToFile("Mouse Right Button down command is executed");
				break;
			case CM_RBUTTONUP:
				retVal = service.SendRButtonUp(xPos, yPos);
				FileLogger::LogToFile("Mouse Right Button down command is executed");
				break;
			case CM_MOUSEMOVE:
				char strText[100];
				sprintf(strText, "xPos: %d, yPos: %d", xPos, yPos);
				logger.Info(strText);
				retVal = service.SendMouseMove(xPos, yPos);
				FileLogger::LogToFile("Mouse Move command is executed");
				break;
			case CM_KEYDOWN:
				retVal = service.SendKeyDown(dm.message); //message will be the key pressed
				sprintf(strText, "RetVal for Send Key Down: %d", retVal);
				FileLogger::LogToFile(strText);
				break;
			case CM_KEYUP:
				service.SendKeyUp(dm.message); //message will be the key pressed
				break;
			case CM_CLOSESESSION:
				if (session != NULL) {
					session->audio_video->Stop();
					string msg = "Session is being closed now. Other joinees should not be able to view the Remote Desktop";
					logger.Info(msg);
					FileLogger::LogToFile(msg);
					HANDLE handle = GetCurrentProcess();
					TerminateProcess(handle, 0);
					FileLogger::LogToFile("Application is closing now!!!");
				}
				break;
			case CM_CTRLALTDEL:
				AdvancedKeys::SendSAS();
				break;
			default:
				logger.Info(dm.message);
				break;
			}
		}
	}
};

//Create an Web API that has an end point that calls the Meeting Lambda  API and that will be triggered from the CPP App.
// ________________________________________________
//
// CreateSession
//
// PURPOSE: 
// Helper Function to create a Meeting Session.
//
// ARG:
// STRING: The Meeting ID to create. 
// RETURN VALUE:
// shared poitner of MeetingSession: A Valid Meeting session created based on the Meeting Id. 
// ________________________________________________
 std::shared_ptr<chime::MeetingSession> createSession(string meetingId, string json) {
	std::unique_ptr<chime::Logger> logger = std::make_unique<chime::ConsoleLogger>(chime::LogLevel::kDebug);

	string baseUrl = "https://pulud6u8je.execute-api.us-east-1.amazonaws.com/Prod/join?m=" + meetingId + "&e=technician";
	//string baseUrl = "https://localhost:44313/" + meetingId; //Wrapper REST API that will internally call the Actual API provided under AWS Lambda. 

	// Replace 'your_url' with the actual URL you want to call
	auto response = cpr::Get(cpr::Url{ baseUrl });
	//auto response = json;//to be modified for integration...

	// Parse the JSON response
	auto json_response = nlohmann::json::parse(response.text);//should be set to response for integration. 

	// Deserialize JSON into C++ structs
	ResponseData data = json_response.get<ResponseData>();

	// Now you can use 'data' as a C++ object
	FileLogger::LogToFile( "Attendee ID: " + data.attendee.AttendeeId);
	FileLogger::LogToFile( "Meeting ID: " + data.meeting.MeetingId);
	FileLogger::LogToFile( "Audio Host URL: " +  data.meeting.MediaPlacement.AudioHostUrl);
	FileLogger::LogToFile( "Signaling URL: " + data.meeting.MediaPlacement.SignalingUrl);
	FileLogger::LogToFile( "External User ID: " + data.attendee.ExternalUserId);
	FileLogger::LogToFile( "Join Token: " + data.attendee.JoinToken);

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

 // ________________________________________________
 //
 // joinAsFirstAttendee
 //
 // PURPOSE: 
 // Helper Function to Join the Meeting with MeetingId.
 //
 // ARG:
 // STRING: The Meeting ID to create. 
 // 
 // RETURN VALUE:
 // VOID
 // ________________________________________________
void joinAsFirstAttendee(string meetingId, string json) {
	session = createSession(meetingId, json);
	FileLogger::LogToFile("Meeting Session has been created");
	MyAudioVideoObserver my_audio_video_observer = MyAudioVideoObserver();

	session->audio_video->AddAudioVideoObserver(&my_audio_video_observer);

	std::shared_ptr<FakeFrameVideoSource> fake_frame_source = std::make_shared<FakeFrameVideoSource>();

	chime::LocalVideoConfiguration local_video_config;
	local_video_config.modality = chime::Modality::kContent;
	local_video_config.codecPreferences = { chime::VideoCodecCapability::H264_MAIN_PROFILE };
	session->audio_video->AddLocalVideo(fake_frame_source, local_video_config);

	// Start generating frames in a separate thread
	// Directly call StartGeneratingFrames without creating a new thread
	session->audio_video->Start();
	service.StartInputProcessingThread();
	//std::thread frameGenerationThread(&FakeFrameVideoSource::StartGeneratingFrames, fake_frame_source);
	//FileLogger::LogToFile("Frames have started to be sent");
	fake_frame_source->StartGeneratingFrames(0);
	//StartGeneratingFrame -> CreateVideoFrame -> CaptureScreen -> DirectX / BitBlt -> Screen Shot.
	Sleep(5000);
	//FileLogger::LogToFile("Frames are getting generated");
	std::cout << "Press any key to end the meeting" << std::endl;
	std::cin.get();
}


void joinAsSecondAttendee(string meetingId, string jsonData) {
	DisplayInformation info;
	info.EnumerateAndSelectMonitor(1);
	if (info.GetNoOfMonitors() == 1) {
		cout << "No Secondary Monitor Found" << endl;
		return;
	}
	session2 = createSession(meetingId, jsonData);
	FileLogger::LogToFile("2nd Meeting Session has been created");
	MyAudioVideoObserver my_audio_video_observer = MyAudioVideoObserver();

	//session2->audio_video->AddAudioVideoObserver(&my_audio_video_observer);

	std::shared_ptr<FakeFrameVideoSource> fake_frame_source = std::make_shared<FakeFrameVideoSource>();

	chime::LocalVideoConfiguration local_video_config;
	local_video_config.modality = chime::Modality::kContent;
	local_video_config.codecPreferences = { chime::VideoCodecCapability::H264_MAIN_PROFILE };
	session2->audio_video->AddLocalVideo(fake_frame_source, local_video_config);

	// Start generating frames in a separate thread
	// Directly call StartGeneratingFrames without creating a new thread
	session2->audio_video->Start();
	service.StartInputProcessingThread();
	//std::thread frameGenerationThread(&FakeFrameVideoSource::StartGeneratingFrames, fake_frame_source);
	//FileLogger::LogToFile("Frames have started to be sent");
	fake_frame_source->StartGeneratingFrames(1);
	Sleep(5000);
	//FileLogger::LogToFile("Frames are getting generated");
	std::cout << "Press any key to end the meeting" << std::endl;
	std::cin.get();
}
int main(int argc, char* argv[])
{

	if (argv[1] == NULL) {
		FileLogger::LogToFile("No Meeting Details are set, cannot start the App");
		std::thread primaryThread(&joinAsFirstAttendee, "801445", "");//setting the 1st meeting as 
		std::thread secondThread(&joinAsSecondAttendee, "801446", "");//setting the 2nd meeting as 
		primaryThread.join();
	}
	else {
		invokeUsingCmdArgs(argv);
	}
	std::cout << "Press any key to end the meeting" << std::endl;
	std::cin.get();
}

void invokeUsingCmdArgs(char* argv[])
{
	//We are not taking cmdline args for the monitor index. 
	if (argv[1] != NULL && argv[2] != NULL && argv[3] != NULL)
	{
		std::string meetingId = argv[1];//for meetingId
		std::string primaryJsonData = argv[2];//for meetingdetails passed as json. 
		std::string secondaryJsonData = argv[3];//for meetingdetails passed as json. 
		logger.Info(meetingId);
		FileLogger::LogToFile("Joined with CommandLine arg");
		joinAsFirstAttendee(meetingId, primaryJsonData);
	}
	else {
		std::string msg = "Invalid Args, Cannot start the EXE";
		FileLogger::LogToFile(msg);
		logger.Error(msg);
	}
}

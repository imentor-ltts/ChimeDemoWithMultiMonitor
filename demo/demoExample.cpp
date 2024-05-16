#include <cpr\cpr.h>
#include <nlohmann\json.hpp>
#include<iostream>
using namespace std;

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

void createSession(string meetingId) {
	
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

}

void main() {
	createSession("801445");
};

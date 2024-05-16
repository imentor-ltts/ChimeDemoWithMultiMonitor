// --------------------------------------------------------------------------------------------------------------------
// <copyright file="EventMessage.h" company="Siemens Ultrasound">
// Copyright(c) Since 2022 by Siemens Healthineers
// All Rights Reserved.
// No part of this software may be reproduced or transmitted in any
// form or by any means including photocopying or recording without
// written permission of the copyright owner.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------
#pragma once
#include<Windows.h>
#include<iostream>
#include "nlohmann/json.hpp"

chime::ConsoleLogger logger;
namespace EventMessages {
	
	/// <summary>
	/// Represents the input mouse or keyboard message sent by the joinee
	/// </summary>
	struct DataMessage {
		int eventId = 0;
		float xPos = 0;
		float yPos = 0;
		WORD keyCode = -1;
		std::string message = "";
		int monitor = 1;

		//Represent the contents of the DataMessage as string.
		/*std::string toString() {
			char strText[100];
			sprintf_s(strText, 100 + message.size(), "EventId: %d, xPos: %f, yPos: %f, keyCode: %d, message: %s");
			return strText;
		}*/
	};

	// ________________________________________________
	 //
	 // from_json
	 //
	 // PURPOSE: 
	 // Extracts the DataMessage from the JSON Data obtained thru Data messages of Chime SDK. 
	 //
	 // ARGS:
	 // JSON: The JSON data that has to be converted. 
	 // DATAMESSAGE: The Struct that will contain the JSON transformed data. 
	 // RETURN VALUE:
	 // VOID
	 // ________________________________________________
	 //
	void from_json(const nlohmann::json& jsonData, DataMessage& message) {
		jsonData.at("eventId").get_to(message.eventId);
		jsonData.at("xPos").get_to(message.xPos);
		jsonData.at("yPos").get_to(message.yPos);
		jsonData.at("keyCode").get_to(message.keyCode);
		jsonData.at("message").get_to(message.message);
		jsonData.at("monitor").get_to(message.monitor);
	}

	enum ButtonAction {
		MouseDown, MouseUp
	};
}
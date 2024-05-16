const videoSizeMultiplier = 0.8;

let baseUrl = "https://pulud6u8je.execute-api.us-east-1.amazonaws.com/Prod/";
let apiUrl = baseUrl + "create"; // assumes meetingInfo is using the same API GW

let meetingSession;
let count = 0;
let screenTileAssociation = []

//*******************************************************************************
// Function:  getMeetingInfo(meetingId)
// Returns: Meeting and Attendee Info
// If meeting exists, returns existing meeting info,
// else starts new meeting and returns that info
// Always creates a new Attendee (limit 250)
//*******************************************************************************
async function getMeetingInfo(meetingId) {
    const options = {
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
        }
    };
    const url = `${apiUrl}?m=${meetingId}&path=/createMeeting`;
    const response = await fetch(url, options);
    return await response.json();
}

//*******************************************************************************
// Function:  joinMeetingInfo(meetingId, joineename)
// meetingId is the Id of the meeting shared. 
// joinee name : Name entered by the user before he clicks the join button
// Returns: Meeting and Attendee Info of the joined person
// If meeting exists, returns existing meeting info,
// else starts new meeting and returns that info
// Always gets a new Attendee (limit 250)
//*******************************************************************************
async function joinMeetingInfo(meetingId, joineename) {
    const url = `${baseUrl}/join?m=${meetingId}&e=${joineename}`;
    const options = {
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
        }
    };
    const response = await fetch(url, options);
    return await response.json();
}


/**
 * Listener for Handling Data Messages
 * @param {any} dataMessage: Message sent by the Attendee
 */
function onDataMessageReceived(dataMessage) {
    const data = dataMessage.data;
    console.log(data);
}

/**
 * Provide an observer object that defines the Event Handlers for various events that occur during the chime meeting.
 * @returns An observer object with all the event handlers that we wish to handle
 */
function createObserver() {
    const videoElementScreenShare = document.getElementById('video-tile-screen-share');

    const observer = {
        audioVideoDidStart: () => {
           
            console.log("audioVideoDidStart fired")
        },

        videoTileDidUpdate: tileState => {
            if (!tileState.boundAttendeeId) {
                return;
            }
            if (tileState.localTile) {
                console.log("local tile")
            } else if (tileState.isContent) {
                if (!document.getElementById(tileState.tileId)) {
                    const node = document.createElement("video");
                    node.id = tileState.tileId;
                    videoElementScreenShare.appendChild(node);
                    const videoElementNew = document.getElementById(tileState.tileId);
                    meetingSession.audioVideo.bindVideoElement(tileState.tileId, videoElementNew);
                    var screen = "Originator";
                    if (screen != null & screen != '') {
                        var monitorNum = parseInt(screen.split(':')[1])
                        screenTileAssociation[tileState.tileId] = monitorNum
                    }
                }
            }
        },
        videoTileWasRemoved: tileId => {
            const videoElementRemoved = document.getElementById(tileId);
            videoElementRemoved?.remove();
        },
        audioVideoDidStop: async sessionStatus => {  // v3
            await meetingSession.audioVideo.stopAudioInput();
            meetingSession.deviceController.destroy();
        },
        attendeeDidJoin: (attendeeId) => {
            debugger;
            console.log(`Attendee ${attendeeId} joined`);
        },
        attendeeDidLeave: (attendeeId) => {
            debugger;
            console.log(`Attendee ${attendeeId} left`);
        },
    };
    return observer;
}

/**
 * Event Handler for Join Meeting
 * @param {any} meetingId: Meeting ID to join that is generated.
 * @param {any} joineeName: Name of the person who joins the meeting.
 */
async function onJoinMeetingInfo(meetingId, joineeName) {
    const data = await joinMeetingInfo(meetingId, joineeName); // Fetch the meeting and attendee data
    console.log('Success getting meeting info ', data);
    const meeting = data.Meeting;
    const attendee = data.Attendee;
    var slAttendees = document.getElementById("slAttendees");
    if (data.Attendee.length == undefined) {
        var option = document.createElement("option"),
            txt = document.createTextNode(data.Attendee.ExternalUserId);
        option.appendChild(txt);
        option.setAttribute("value", data.Attendee.AttendeeId);
        slAttendees.insertBefore(option, slAttendees.lastChild);
    }
    else {
        for (var i = 0; i < data.Attendee.length; i++) {
            var option = document.createElement("option"),
                txt = document.createTextNode(data.Attendee.ExternalUserId[i]);
            option.appendChild(txt);
            option.setAttribute("value", data.Attendee.AttendeeId[i]);
            slAttendees.insertBefore(option, slAttendees.lastChild);
        }
    }
    
    const logger = new ChimeSDK.ConsoleLogger('MyLogger');
    const deviceController = new ChimeSDK.DefaultDeviceController(logger);
    const configuration = new ChimeSDK.MeetingSessionConfiguration(meeting, attendee);
    meetingSession = new ChimeSDK.DefaultMeetingSession(configuration, logger, deviceController);
    console.log('meetingSession ', meetingSession);

    const presentAttendeeId = meetingSession.configuration.credentials.attendeeId;
    console.log(`Present Attendee ID: ${presentAttendeeId}`);

    const browserBehaviour = new ChimeSDK.DefaultBrowserBehavior();
    console.log('supportSetSinkId is ', browserBehaviour.supportsSetSinkId());

    try {

        await meetingSession.audioVideo.startAudioInput(null);  // v3
        console.log('empty set for chooseAudioInputDevice');

    } catch (err) {
        // handle error - unable to acquire video or audio device perhaps due to permissions blocking or chromium bug on retrieving device label
        // see setupDeviceLabelTrigger() on https://github.com/aws/amazon-chime-sdk-js/blob/main/demos/browser/app/meetingV2/meetingV2.ts
        console.log('Try Catch Error - unable to acquire device - ', err);
    }
    meetingSession.audioVideo.realtimeSubscribeToReceiveDataMessage("Basic Demo", onDataMessageReceived);
    meetingSession.audioVideo.realtimeSubscribeToAttendeeIdPresence((attendeeId, presence) => {
        debugger;
        if (presence)
            console.log(`${attendeeId} has joined the call!!!`);
        else {
            console.log(`${attendeeId} has left the call`);
        }
    })

    const observer = createObserver();
    meetingSession.audioVideo.addObserver(observer);
    meetingSession.audioVideo.start();
    meetingSession.audioVideo.startLocalVideoTile();
    
}



/**
 * Event Handler function to create new Meeting Session. This is created by the Originator of the Meeting
 * @param {any} meetingId: A valid Non-Negative number to identify the meeting 
 */
async function createMeeting(meetingId) {
    document.oncontextmenu = function () {
        return false;
    }
    const data = await getMeetingInfo(meetingId); // Fetch the meeting and attendee data
    console.log('Success getting meeting info ', data);

    const meeting = data.Meeting;
    const attendee = data.Attendee;

    document.getElementById('meetingId').value = meeting.MeetingId;
    document.getElementById('attendeeId').value = attendee.AttendeeId;
    document.getElementById('externalMeetingId').value = meeting.ExternalMeetingId;
    
    const logger = new ChimeSDK.ConsoleLogger('MyLogger');
    const deviceController = new ChimeSDK.DefaultDeviceController(logger);
    console.log('deviceController', deviceController);

    const configuration = new ChimeSDK.MeetingSessionConfiguration(meeting, attendee);
    console.log('configuration ', configuration);

    meetingSession = new ChimeSDK.DefaultMeetingSession(configuration, logger, deviceController);
    console.log('meetingSession ', meetingSession);

    const presentAttendeeId = meetingSession.configuration.credentials.attendeeId;
    console.log('presentAttendeeId - ', presentAttendeeId);

    const browserBehaviour = new ChimeSDK.DefaultBrowserBehavior();
    console.log('supportSetSinkId is ', browserBehaviour.supportsSetSinkId());

    try {

        await meetingSession.audioVideo.startAudioInput(null);  // v3
        console.log('empty set for chooseAudioInputDevice');

    } catch (err) {
        // handle error - unable to acquire video or audio device perhaps due to permissions blocking or chromium bug on retrieving device label
        // see setupDeviceLabelTrigger() on https://github.com/aws/amazon-chime-sdk-js/blob/main/demos/browser/app/meetingV2/meetingV2.ts
        console.log('Try Catch Error - unable to acquire device - ', err);
    }

    const observer = createObserver();
    meetingSession.audioVideo.addObserver(observer);
    meetingSession.audioVideo.realtimeSubscribeToReceiveDataMessage("Basic Demo", onDataMessageReceived);
    meetingSession.audioVideo.realtimeSubscribeToAttendeeIdPresence((attendeeId, presence, userId) => {
        debugger;
        if (presence) {
            console.log(`${userId} has joined the call!!!`);
            document.getElementById("pStatus").innerHTML = `${userId} has joined the call!!!<br/>`;
        }else {
            console.log(`${userId} has left the call`);
            document.getElementById("pStatus").innerHTML += `${userId} has left the call!!!<br/>`;
        }
    })

    meetingSession.audioVideo.start();
    meetingSession.audioVideo.startLocalVideoTile();
}

/*********************************************************************
 * Helper function for getting the Mouse Coordinates relative to the Screen
 * @param {any} event
 * @returns
 ***********************************************************/
function getCoordinates(e) {
    const rect = e.target.getBoundingClientRect();
    const percentX = e.clientX - rect.left; //x position within the element.
    const percentY = e.clientY - rect.top;  //y position within the element.
    return [percentX, percentY];
}


function handleMouseMovement(event) {
    // Get mouse coordinates relative to the div
    const [percentX, percentY] = getCoordinates(event);
    let mouseAction = {
        eventId: 'WM_MOUSEMOVE',
        xPos: percentX,
        yPos: percentY,
        monitor: 1,
        keyCode: 0,
        message: ''
    }
    if (meetingSession == undefined) {
        console.log("Meeting is yet to start");
    }else
        meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", mouseAction);
}


function handleMouseDown(event) {
    const [percentX, percentY] = getCoordinates(event);
    let mouseAction = {
        eventId: 'WM_LBUTTONDOWN',
        xPos: percentX,
        yPos: percentY,
        monitor: 1,
        keyCode: 0,
        message: ''
    }
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", mouseAction);
}

function handleMouseUp(event) {
    const [percentX, percentY] = getCoordinates(event);
    let mouseAction = {
        eventId: 'WM_LBUTTONUP',
        xPos: percentX,
        yPos: percentY,
        monitor: 1,
        keyCode: 0,
        message: ''
    }
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", mouseAction);
}
function handlePasteEvent(event) {
    event.preventDefault();
    // Get the pasted text
    try {
        navigator.clipboard.readText()
            .then(text => {
                let keyAction = {
                    eventId: 'paste',
                    monitor: 1,
                    xPos: 0,
                    yPos: 0,
                    keyCode:-1,
                    message: text
                };
                console.log(keyAction);
                meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", keyAction);
            })
    } catch (e) {
        console.log(e.message);
    }
}

let clipboardTextLength = 0;
function getClipboardText() {
    navigator.clipboard.readText()
        .then(text => {
            clipboardTextLength = text.length;
            
        })
        .catch(err => {
            console.error('Failed to read clipboard contents: ', err);
        });
    
}
function handleAltKeyDown(event) {
    let keyAction = {
        eventId: 'AltDown',
        monitor: 1,
        xPos: 0,
        yPos: 0,
        message: event.key,
        keyCode: event.keyCode
    };

    console.log(keyAction);
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", keyAction);
}
function handleKeyDown(event) {
    event.preventDefault();
    getClipboardText();
    if (event.ctrlKey && event.key === 'v' && clipboardTextLength >0) {
        handlePasteEvent(event);
        return;
    } 
    if (event.ctrlKey && (event.key === 'c' || event.key === 'x')) {
        navigator.clipboard.writeText('')
            .then(() => {
                console.log('Clipboard cleared successfully');
            })
            .catch(err => {
                console.error('Failed to clear clipboard:', err);
            });      
    }
    if (event.altKey) {
        handleAltKeyDown(event);
    }
    
    let keyAction = {
        eventId: 'WM_KEYDOWN',
        monitor: 1,
        xPos: 0,
        yPos: 0,
        message: event.key,
        keyCode:event.keyCode
    };
   
    console.log(keyAction);
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", keyAction);
}

 //Function to handle keyup event
function handleKeyUp(event) {
    event.preventDefault();
    let keyAction = {
        eventId: 'WM_KEYUP',
        monitor: 1,
        xPos: 0,
        yPos: 0,
        message: event.key,
        keyCode: event.keyCode
    };
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", keyAction);
}
function onSendTextMessage(textMessage) {
    let data = {
        eventId: 'textMessage',
        monitor: 1,
        xPos: 0,
        yPos: 0,
        keyCode: -1,
        message: textMessage
    }
    debugger;
    document.getElementById("msgContent").innerHTML += `${textMessage}<br/>`;
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", data);
    return textMessage;
}

function sendSelectedOption(selectedKeys) {
    let specialKeyAction = {
        eventId: selectedKeys,
        monitor: 1,
        xPos: 0,
        yPos: 0,
        message: selectedKeys,
        keyCode: -1
    };
    meetingSession.audioVideo.realtimeSendDataMessage("BasicDemo", specialKeyAction);
}
function handleSpecialKeySelection() {
    var dropdown = document.getElementById('specialKeyDropdown');
    let selectedOption = dropdown.options[dropdown.selectedIndex].value;

    // Trigger an event based on the selected dropdown item
    switch (selectedOption) {
        case 'winR':
            // Event for Option 1
            sendSelectedOption('run');
            console.log('Event triggered for Option 1');
            break;
        case 'winX':
            // Event for Option 2
            sendSelectedOption('showMenus');
            console.log('Event triggered for Option 2');
            break;
        case 'SpKeys1':
            // Event for Option 3
            sendSelectedOption('SpKeys1');
            console.log('Event triggered for Option 3');
            break;
        default:
            console.log('No event defined for this option');
    }
}









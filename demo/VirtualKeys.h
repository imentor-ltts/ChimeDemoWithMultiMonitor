#pragma once
#include<Windows.h>
#include<iostream>

/// <summary>
/// Represents the virtual keys of the OS for JS conversion.
/// </summary>
enum VirtualKeys {
	Down = VK_DOWN,
	Up = VK_UP,
	Left = VK_LEFT,
	Right = VK_RIGHT,
	Enter = VK_RETURN,
	Esc = VK_ESCAPE,
	Menu = 12,
	Control = VK_CONTROL,
	Shift = VK_SHIFT,
	PAUSE = VK_PAUSE,
	BREAK = VK_PAUSE,
	Backspace = VK_BACK,
	Tab = VK_TAB,
	CapsLock = VK_CAPITAL,
	Delete = VK_DELETE,
	Home = VK_HOME,
	End = VK_END,
	PageUp = VK_PRIOR,
	PageDown = VK_NEXT,
	NumLock = VK_NUMLOCK,
	Insert = VK_INSERT,
	ScrollLock = VK_SCROLL,
	F1 = VK_F1,
	F2 = VK_F2,
	F3 = VK_F3,
	F4 = VK_F4,
	F5 = VK_F5,
	F6 = VK_F6,
	F7 = VK_F7,
	F8 = VK_F8,
	F9 = VK_F9,
	F10 = VK_F10,
	F11 = VK_F11,
	F12 = VK_F12,
	LWin = VK_LWIN,
	RWin = VK_RWIN
};



struct ModifierState {
	bool Pressed;
	bool Toggled;
};


// ________________________________________________
//
// ConvertJSKeyToVirtualKey
//
// PURPOSE: 
// Converts the JavaScript keycode to Virtual KeyCode of the Windows OS.
//
// RETURN VALUE:
// BOOL : The Value indicates the operation was successful and returns FALSE if the Conversion has failed.
// ________________________________________________
//
bool ConvertJSKeyToVirtualKey(std::string key, VirtualKeys& vKey) {
	if (key == "ArrowDown") { vKey = VirtualKeys::Down; return true; }
	if (key == "ArrowUp") { vKey = VirtualKeys::Up; return true; }
	if (key == "ArrowLeft") { vKey = VirtualKeys::Left; return true; }
	if (key == "ArrowRight") { vKey = VirtualKeys::Right; return true; }
	if (key == "Enter") { vKey = VirtualKeys::Enter; return true; }
	if (key == "Escape") { vKey = VirtualKeys::Esc; return true; }
	if (key == "Alt") { vKey = VirtualKeys::Menu; return true; }
	if (key == "Control") { vKey = VirtualKeys::Control; return true; }
	if (key == "Shift") { vKey = VirtualKeys::Shift; return true; }
	if (key == "PAUSE") { vKey = VirtualKeys::PAUSE; return true; }
	if (key == "BREAK") { vKey = VirtualKeys::PAUSE; return true; }
	if (key == "Backspace") { vKey = VirtualKeys::Backspace; return true; }
	if (key == "Tab") { vKey = VirtualKeys::Tab; return true; }
	if (key == "CapsLock") { vKey = VirtualKeys::CapsLock; return true; }
	if (key == "Delete") { vKey = VirtualKeys::Delete; return true; }
	if (key == "Home") { vKey = VirtualKeys::Home; return true; }
	if (key == "End") { vKey = VirtualKeys::End; return true; }
	if (key == "PageUp") { vKey = VirtualKeys::PageUp; return true; }
	if (key == "PageDown") { vKey = VirtualKeys::PageDown; return true; }
	if (key == "NumLock") { vKey = VirtualKeys::NumLock; return true; }
	if (key == "Insert") { vKey = VirtualKeys::Insert; return true; }
	if (key == "ScrollLock") { vKey = VirtualKeys::ScrollLock; return true; }
	if (key == "F1") { vKey = VirtualKeys::F1; return true; }
	if (key == "F2") { vKey = VirtualKeys::F2; return true; }
	if (key == "F3") { vKey = VirtualKeys::F3; return true; }
	if (key == "F4") { vKey = VirtualKeys::F4; return true; }
	if (key == "F5") { vKey = VirtualKeys::F5; return true; }
	if (key == "F6") { vKey = VirtualKeys::F6; return true; }
	if (key == "F7") { vKey = VirtualKeys::F7; return true; }
	if (key == "F8") { vKey = VirtualKeys::F8; return true; }
	if (key == "F9") { vKey = VirtualKeys::F9; return true; }
	if (key == "F10") { vKey = VirtualKeys::F10; return true; }
	if (key == "F11") { vKey = VirtualKeys::F11; return true; }
	if (key == "F12") { vKey = VirtualKeys::F12; return true; }
	if (key == "Meta" || key == "Win") { vKey = VirtualKeys::LWin; return true; }
	if (vKey == 0) return false;
	return false;
}


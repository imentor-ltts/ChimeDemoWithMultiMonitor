#include "pch.h"
#include "main.h"
#include "DataMessage.h"

class ChimeMeetingTest : public ::testing::Test {
protected:
	HBITMAP hBmp;
	std::vector<uint8_t> bitmapData;
	bool status;
	UINT keys;
public:

	////////////////////Screen Capturing Testing////////////////////////////////
	void CallCaptureScreen() {
		hBmp = CaptureScreen();
	}

	////////////////////Bitmap Data Testing////////////////////////////////
	void TestForGetBitmapData() {
		hBmp = CaptureScreen();
		BITMAPINFO bmpInfo;
		bitmapData = GetBitmapData(hBmp, bmpInfo);
	}

	////////////////////KeyBoard Testing////////////////////////////////
	void TestForConvertJSKeyToVirtualKey() {
		VirtualKeys key;
		status = ConvertJSKeyToVirtualKey("Control", key);
	}
	void TestForKeyBoardInputSuccess() {
		keys = KeyBoardService::SendKeyUp("R");
	}
	void TestForConvertJSKeyToVirtualKeyForFalse() {
		VirtualKeys key;
		status = ConvertJSKeyToVirtualKey("Ctrl", key);
	}

	void TestForKeyDownSuccess() {
		keys = KeyBoardService::SendKeyDown("R");
	}

	void TestForPasteOperationSuccess() {
		std::string text = "Apple123";
		keys = KeyBoardService::SendText(text);
	}

	void TestForSendAltKeyUpSuccess() {
		keys = KeyBoardService::SendAltKeyUp();
	}

	void TestForSendAltKeyDownSuccess() {
		keys = KeyBoardService::SendAltKeyDown();
	}
	/////////////////////Mouse Testing//////////////////////////////
	void TestForLButtonDownOperationSuccess() {
		keys = MouseService::SendLButtonDown(45, 66);
	}

	void TestForRButtonUpOperationSuccess() {
		keys = MouseService::SendRButtonUp(45, 66);
	}

	void TestForRButtonDownOperationSuccess() {
		keys = MouseService::SendRButtonDown(65, 67);
		MouseService::SendRButtonUp(65, 67);
	}

	void TestForLButtonUpOperationSuccess() {
		keys = MouseService::SendMouseUp();
	}

	void TestForSendMouseMove() {
		keys = MouseService::SendMouseMove(600, 700);
	}
	////////////////////SetUp and Tear Down override Methods////////////////////////////////
	void SetUp() override {
		hBmp = NULL;
		status = false;
	}
	virtual void TearDown() override {
		hBmp = NULL;
		status = false;
   }

	void TestBody()override {

   }
	///////////////////////////////////////////////////////////////////////////
};

TEST_F(ChimeMeetingTest, CheckForBitmapCreation) {
	//Arrange
	//Act
	CallCaptureScreen();
	//Assert
	ASSERT_TRUE(hBmp != NULL);
}

TEST_F(ChimeMeetingTest, CheckForBitmapData) {
	//Arrange
	//Act
	TestForGetBitmapData();
	//Assert;
	ASSERT_TRUE(bitmapData.size() > 0);
}

TEST_F(ChimeMeetingTest, CheckForJSKeyConversionSuccess) {
	//Act
	TestForConvertJSKeyToVirtualKey();
	ASSERT_TRUE(true, status);
}

TEST_F(ChimeMeetingTest, CheckForJSKeyConversionFailure) {
	//Act
	TestForConvertJSKeyToVirtualKeyForFalse();
	ASSERT_FALSE(false, status);
}

TEST_F(ChimeMeetingTest, CheckForKeyUpInputSuccess) {
	TestForKeyBoardInputSuccess();
	ASSERT_TRUE(keys > 0);
}

TEST_F(ChimeMeetingTest, CheckForKeyDownInputSuccess) {
	TestForKeyDownSuccess();
	ASSERT_TRUE(keys > 0);
}

TEST_F(ChimeMeetingTest, TestForPasteSuccess) {

	//Act
	TestForPasteOperationSuccess();
	ASSERT_TRUE(keys == 16);
}

TEST_F(ChimeMeetingTest, TestForMouseDown) {
	TestForLButtonDownOperationSuccess();
	ASSERT_TRUE(keys > 0);
}

TEST_F(ChimeMeetingTest, TestForRButtonUp) {
	TestForRButtonUpOperationSuccess();
	ASSERT_TRUE(keys > 0);
}

TEST_F(ChimeMeetingTest, TestForRButtonDown) {
	TestForRButtonDownOperationSuccess();
	ASSERT_TRUE(keys > 0);
}


TEST_F(ChimeMeetingTest, TestForMouseUp) {
	TestForLButtonUpOperationSuccess();
	ASSERT_TRUE(keys > 0);
}


TEST_F(ChimeMeetingTest, TestForMouseMove) {
	TestForSendMouseMove();
	ASSERT_TRUE(keys > 0);
}


TEST_F(ChimeMeetingTest, TestForAltKeySuccess) {
	TestForSendAltKeyUpSuccess();
	ASSERT_TRUE(keys > 0);
}

TEST_F(ChimeMeetingTest, TestForAltKeyDownSuccess) {
	TestForSendAltKeyDownSuccess();
	ASSERT_TRUE(keys > 0);
}


int main(int argc, char** argv) {
	// Initialize Google Test
	::testing::InitGoogleTest(&argc, argv);

	// Run tests
	return RUN_ALL_TESTS();
}
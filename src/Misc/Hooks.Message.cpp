#include <MessageListClass.h>
#include <WWMouseClass.h>

#include <Ext/Scenario/Body.h>

namespace MessageTemp
{
	bool OnMessages = false;
	bool NewMsgList = false;
}

bool MouseIsOverMessageLists()
{
	const auto pMousePosition = &WWMouseClass::Instance->XY1;
	const auto pMessages = ScenarioExt::Global()->NewMessageList.get();

	if (TextLabelClass* pText = pMessages->MessageList)
	{
		if (pMousePosition->Y >= pMessages->MessagePos.Y && pMousePosition->X >= pMessages->MessagePos.X && pMousePosition->X <= pMessages->MessagePos.X + pMessages->Width)
		{
			const int textHeight = pMessages->Height;
			int height = pMessages->MessagePos.Y;

			for ( ; pText; pText = static_cast<TextLabelClass*>(pText->GetNext()))
				height += textHeight;

			if (pMousePosition->Y < (height + 2))
				return true;
		}
	}

	return false;
}

DEFINE_HOOK(0x69300B, ScrollClass_MouseUpdate_SkipMouseActionUpdate, 0x6)
{
	if (Phobos::Config::MessageDisplayInCenter)
		MessageTemp::OnMessages = MouseIsOverMessageLists();

	return 0;
}

DEFINE_HOOK(0x4F4583, GScreenClass_DrawCurrentSelectInfo, 0x6)
{
	MessageTemp::NewMsgList = true;
	ScenarioExt::Global()->NewMessageList->Draw();
	MessageTemp::NewMsgList = false;

	return 0;
}

DEFINE_HOOK(0x55DDA0, MainLoop_FrameStep_NewMessageListManage, 0x5)
{
	if (!MessageTemp::OnMessages)
	{
		if (const auto pList = ScenarioExt::Global()->NewMessageList.get())
			pList->Manage();
	}

	return 0;
}

DEFINE_HOOK(0x5D3BA0, MessageListClass_AddMessage_InCenter, 0x6)
{
	if (*R->ESP<int*>() == 0x6DE127) // TActionClass::Execute
	{
		if (const auto pList = ScenarioExt::Global()->NewMessageList.get())
			R->ECX(pList);
	}

	return 0;
}

DEFINE_HOOK(0x4A8BCE, DisplayClass_Set_View_Dimensions, 0x5)
{
	if (Phobos::Config::MessageDisplayInCenter)
	{
		const auto& pScenarioExt = ScenarioExt::Global();

		if (!pScenarioExt->NewMessageList) // Load game
			pScenarioExt->NewMessageList = std::make_unique<MessageListClass>();

		const auto& rect = DSurface::ViewBounds;
		const auto sideWidth = rect.Width / 6;
		const auto width = rect.Width - (sideWidth * 2);
		const auto pList = pScenarioExt->NewMessageList.get();

		// Except for X and Y, they are all original values
		pList->Init((rect.X + sideWidth), (rect.Height - rect.Height / 8 - 120), 6, 98, 18, -1, -1, 0, 20, 98, width);
		pList->SetWidth(width);
	}

	return 0;
}

DEFINE_HOOK(0x684AD3, UnknownClass_sub_684620_InitMessageList, 0x5)
{
	if (Phobos::Config::MessageDisplayInCenter)
	{
		const auto& pScenarioExt = ScenarioExt::Global();

		if (!pScenarioExt->NewMessageList) // Start game
			pScenarioExt->NewMessageList = std::make_unique<MessageListClass>();

		const auto& rect = DSurface::ViewBounds;
		const auto sideWidth = rect.Width / 6;
		const auto width = rect.Width - (sideWidth * 2);
		const auto pList = pScenarioExt->NewMessageList.get();

		// Except for X and Y, they are all original values
		pList->Init((rect.X + sideWidth), (rect.Height - rect.Height / 8 - 120), 6, 98, 18, -1, -1, 0, 20, 98, width);
	}

	return 0;
}

DEFINE_HOOK(0x623A9F, DSurface_sub_623880_DrawBitFontStrings, 0x5)
{
	if (!MessageTemp::NewMsgList)
		return 0;

	enum { SkipGameCode = 0x623AAB };

	GET(RectangleStruct* const, pRect, EAX);
	GET(DSurface* const, pSurface, ECX);
	GET(const int, height, EBP);

	pRect->Height = height;
	auto black = ColorStruct { 0, 0, 0 };
	auto trans = (MessageTemp::OnMessages || ScenarioClass::Instance->UserInputLocked) ? 80 : 40;
	pSurface->FillRectTrans(pRect, &black, trans);

	return SkipGameCode;
}

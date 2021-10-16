#include "UIManager.hpp"
#include "../Timer.hpp"
#include "../Natives.hpp"

#include "../Memory.h"

#pragma comment(lib, "winmm.lib")
#include <timeapi.h>

#include"../Enums.h"

int id = 0; // there fitsr id of your pic (0 or 1) 
int customtimer;
const char* pic_id;
std::string placeholderStr;

namespace Big::UserInterface
{

	void UIManager::Draw_Side_Scroll(AbstractSubmenu* sub) {
	//Get number of options


		float tNumOfOpts = sub->GetNumOptions();
		float percentage =0.f;
		float selected = sub->GetSelectedOption();


		int visibleOpts = tNumOfOpts;
		std::ostringstream yeet;
		
		if (tNumOfOpts > m_OptionsPerPage) {
			percentage = ((selected+1) /tNumOfOpts);
			visibleOpts = m_OptionsPerPage;
		}

		int draw_left = -1; //-1 is left
		if (!isLeftScroller) {
			draw_left = 1;
		}

		//top
		DrawRect(
			m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
			m_DrawBaseY - ((m_OptionHeight * visibleOpts) + m_FooterHeight + m_SubmenuBarHeight/4.f),
			0.008f,
			m_SubmenuBarHeight/2.f,
			m_SubmenuBarBackgroundColor);


		//Middle
		DrawRect(
			m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
			m_DrawBaseY - ((m_OptionHeight * visibleOpts)/2.f + m_FooterHeight),
			0.008f,
			m_OptionHeight * visibleOpts,
			m_OptionUnselectedBackgroundColor);

		//bottom
		DrawRect(
			m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
			m_DrawBaseY - m_FooterHeight/4.f - m_FooterHeight/2.f,
			0.008f,
			m_FooterHeight / 2.f,
			m_FooterBackgroundColor);


		float arrowsize = 0.021f;
		auto sizee = GetSpriteScale(arrowsize);
		//Bottom Sprite
		DrawSprite("commonmenu",
			"arrowright",
			m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
			m_DrawBaseY - m_FooterHeight / 4.f - m_FooterHeight / 2.f,
			sizee.x,
			sizee.y,
			{ 255, 255, 255, 255 },
			90);

		DrawSprite("commonmenu",
			"arrowright",
			m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
			m_DrawBaseY - ((m_OptionHeight * visibleOpts) + m_FooterHeight + m_SubmenuBarHeight / 4.f),
			sizee.x,
			sizee.y,
			{ 255, 255, 255, 255 },
			270);


		//Draw the scroller thingo0
		//Maths coulld probably be optimised or explained nicer with variables but oh well...

		float scroller_height = m_OptionHeight ;
		if (tNumOfOpts <= visibleOpts) {
			DrawRect(
				m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
				m_DrawBaseY - m_FooterHeight - (m_OptionHeight * visibleOpts) + (m_OptionHeight * sub->GetSelectedOption()) + scroller_height/2,
				0.008f,
				scroller_height,
				m_OptionSelectedBackgroundColor);
		}
		else {
			//std::ostringstream yeet;
			

			float scroller_Max = m_DrawBaseY - m_FooterHeight - (m_OptionHeight * visibleOpts) + scroller_height/2;
			float scroller_Min = m_DrawBaseY - m_FooterHeight + scroller_height/4;

			float movement_total = scroller_Max - scroller_Min;
			

			float buffer = -m_FooterHeight;

			if (percentage < 0.05f) {
				percentage = 0.05f;
			}

			float percentage_to_move = percentage * movement_total;
			//yeet << "% = " << percentage;

			DrawRect(
				m_PosX + (draw_left * ((m_Width / 2) + ScrollerPadding)),
				scroller_Max - percentage_to_move + buffer,
				0.008f,
				scroller_height,
				m_OptionSelectedBackgroundColor);

			//g_Logger->Info(yeet.str().c_str());

		}
		


		

	}


	void UIManager::OnTick()
	{
		std::lock_guard lock(m_Mutex);

		if (IsMouseLocked())
		{
			PAD::DISABLE_ALL_CONTROL_ACTIONS(0);
			ShowCursor(true);
			SetCursor(LoadCursorA(NULL, IDC_ARROW));
		
			if (GetMenuRect().IsInBounds(GetMousePos()))
			{
				g_Logger->Info("Kekkeke");
			}
		}

		if (g_UiManager->m_Opened && g_Settings.m_LockMouse)
		{
			PAD::DISABLE_ALL_CONTROL_ACTIONS(0);
		}

		CheckForInput();
		HandleInput();

		if (m_Opened)
		{
			PAD::DISABLE_CONTROL_ACTION(0, 27, true); // Disable phone

			m_DrawBaseY = m_PosY;
			DrawHeader();
			if (!m_SubmenuStack.empty())
			{
				auto sub = m_SubmenuStack.top();
				sub->Reset();
				sub->Execute();

				DrawSubmenuBar(sub);
				
				if (sub->GetNumOptions() != 0)
				{
					std::size_t startPoint = 0;
					std::size_t endPoint = sub->GetNumOptions() > m_OptionsPerPage ? m_OptionsPerPage : sub->GetNumOptions();
					if (sub->GetNumOptions() > m_OptionsPerPage && sub->GetSelectedOption() >= m_OptionsPerPage)
					{
						startPoint = sub->GetSelectedOption() - m_OptionsPerPage + 1;
						endPoint = sub->GetSelectedOption() + 1;
					}

					for (std::size_t i = startPoint, j = 0; i < endPoint; ++i, ++j)
					{
						DrawOption(sub->GetOption(i), i == sub->GetSelectedOption());
					}
				}
				DrawFooter(sub);
				if (DrawScroller) {
					Draw_Side_Scroll(sub);
				}
			}

			
			DrawDescription();
		}
	}

	void UIManager::CheckForInput()
	{
		ResetInput();

		m_OpenKeyPressed = IsKeyPressed(VK_F4) || (PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlScriptRB) && PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendX));
		m_BackKeyPressed = IsKeyPressed(VK_BACK) || PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendCancel);
		m_EnterKeyPressed = IsKeyPressed(VK_RETURN) || PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept);
		m_UpKeyPressed = IsKeyPressed(VK_UP) || PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendUp);
		m_DownKeyPressed = IsKeyPressed(VK_DOWN) || PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendDown);
		m_LeftKeyPressed = IsKeyPressed(VK_LEFT) || PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendLeft);
		m_RightKeyPressed = IsKeyPressed(VK_RIGHT) || PAD::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendRight);
	}

	void UIManager::HandleInput()
	{
		static auto openTicker = GetTickCount();
		if (m_OpenKeyPressed && GetTickCount() - openTicker >= static_cast<std::uint32_t>(m_OpenDelay))
		{
			openTicker = GetTickCount();
			m_Opened ^= true;

			if (m_Sounds)
				AUDIO::PLAY_SOUND_FRONTEND(-1, m_Opened ? "SELECT" : "BACK", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
		}

		static Timer backTimer(0ms);
		backTimer.SetDelay(std::chrono::milliseconds(m_BackDelay));
		if (m_Opened && m_BackKeyPressed && backTimer.Update())
		{
			if (m_Sounds)
				AUDIO::PLAY_SOUND_FRONTEND(-1, "BACK", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);

			if (m_SubmenuStack.size() <= 1)
			{
				m_Opened = false;
			}
			else
			{
				m_SubmenuStack.pop();
			}
		}
		
		if (m_Opened && !m_SubmenuStack.empty())
		{
			auto sub = m_SubmenuStack.top();

			static Timer enterTimer(0ms);
			enterTimer.SetDelay(std::chrono::milliseconds(m_EnterDelay));
			if (m_EnterKeyPressed && sub->GetNumOptions() != 0 && enterTimer.Update())
			{
				if (m_Sounds)
					AUDIO::PLAY_SOUND_FRONTEND(-1, "SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);

				if (auto opt = sub->GetOption(sub->GetSelectedOption()))
				{
					opt->HandleAction(OptionAction::EnterPress);
				}
			}

			static Timer upTimer(0ms);
			upTimer.SetDelay(std::chrono::milliseconds(m_VerticalDelay));
			if (m_UpKeyPressed && sub->GetNumOptions() != 0 && upTimer.Update())
			{
				if (m_Sounds)
					AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);

				sub->ScrollBackward();
			}

			static Timer downTimer(0ms);
			downTimer.SetDelay(std::chrono::milliseconds(m_VerticalDelay));
			if (m_DownKeyPressed && sub->GetNumOptions() != 0 && downTimer.Update())
			{
				if (m_Sounds)
					AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);

				sub->ScrollForward();
			}

			static Timer leftTimer(0ms);
			leftTimer.SetDelay(std::chrono::milliseconds(m_HorizontalDelay));
			if (m_LeftKeyPressed && sub->GetNumOptions() != 0 && leftTimer.Update())
			{
				if (m_Sounds)
					AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);

				if (auto opt = sub->GetOption(sub->GetSelectedOption()))
				{
					opt->HandleAction(OptionAction::LeftPress);
				}
			}

			static Timer rightTimer(0ms);
			rightTimer.SetDelay(std::chrono::milliseconds(m_HorizontalDelay));
			if (m_RightKeyPressed && sub->GetNumOptions() != 0 && rightTimer.Update())
			{
				if (m_Sounds)
					AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET", false);

				if (auto opt = sub->GetOption(sub->GetSelectedOption()))
				{
					opt->HandleAction(OptionAction::RightPress);
				}
			}
		}
	}

	void UIManager::ResetInput()
	{
		m_OpenKeyPressed = false;
		m_BackKeyPressed = false;
		m_EnterKeyPressed = false;
		m_UpKeyPressed = false;
		m_DownKeyPressed = false;
		m_LeftKeyPressed = false;
		m_RightKeyPressed = false;
	}

	void UIManager::DrawHeader()
	{
		switch (m_HeaderType)
		{
		case HeaderType::Static:
			DrawSprite("FrontierTextures",
				"Header (1)",
				m_PosX,
				m_DrawBaseY + (m_HeaderHeight / 2.f) + 0.001f,
				m_Width,
				m_HeaderHeight,
				{ 255, 255, 255, 255 },
				0);
			break;
		case HeaderType::Gradient:
			/*for (std::size_t i = 0; i < (m_HeaderGradientTransparent ? 1 : 20); ++i)
			{
				DrawSprite(
					"aircraft_dials",
					"aircraft_dials_g0",
					m_PosX - (m_HeaderGradientStretch / 2.f),
					m_DrawBaseY + (m_HeaderHeight / 2.f),
					m_Width + m_HeaderGradientStretch + m_HeaderGradientFiller,
					m_HeaderHeight,
					m_HeaderGradientColorRight,
					m_HeaderGradientFlip ? 0.f : 180.f);
				DrawSprite(
					"aircraft_dials",
					"aircraft_dials_g0",
					m_PosX + (m_HeaderGradientStretch / 2.f),
					m_DrawBaseY + (m_HeaderHeight / 2.f),
					m_Width + m_HeaderGradientStretch + m_HeaderGradientFiller,
					m_HeaderHeight,
					m_HeaderGradientColorLeft,
					m_HeaderGradientFlip ? 180.f : 0.f);
			}
			*/
			if ((timeGetTime() - customtimer) > 100) {
				if (id < 72) id++;
				else id = 1;
			}
			placeholderStr = "Header (" + std::to_string(id) + ")";
			pic_id = (placeholderStr).c_str();
			//g_Logger->Info(pic_id);

			DrawSprite("FrontierTextures",
				pic_id,
				m_PosX,
				m_DrawBaseY + (m_HeaderHeight / 2.f) + 0.005f,
				m_Width,
				m_HeaderHeight,
				{ 255, 255, 255, 255 },
				0);
			break;
			

		}

		if (m_HeaderText)
		{
			DrawCenteredText(
				BIGBASE_NAME,
				m_PosX,
				m_DrawBaseY + (m_HeaderHeight / 2.f) - (GetTextHeight(m_HeaderFont, m_HeaderTextSize) / 2.f),
				m_HeaderTextSize,
				m_HeaderFont,
				m_HeaderTextColor,
				false, true);
		}

		m_DrawBaseY += m_HeaderHeight;
	}

	void UIManager::DrawSubmenuBar(AbstractSubmenu* sub)
	{
		char leftText[64] = {};
		std::strncpy(&leftText[0], sub->GetName(), sizeof(leftText) - 1);
		std::transform(std::begin(leftText), std::end(leftText), std::begin(leftText), [](char c) { return static_cast<char>(c); });

		char rightText[32] = {};
		std::snprintf(rightText, sizeof(rightText) - 1, "%zu / %zu", sub->GetSelectedOption() + 1, sub->GetNumOptions());

		DrawRect(
			m_PosX,
			m_DrawBaseY + (m_SubmenuBarHeight / 2.f),
			m_Width, m_SubmenuBarHeight,
			m_SubmenuBarBackgroundColor);
		DrawLeftText(
			&leftText[0],
			m_PosX - (m_Width / m_SubmenuBarPadding),
			m_DrawBaseY + (m_SubmenuBarHeight / 2.f) - (GetTextHeight(m_SubmenuBarFont, m_SubmenuBarTextSize) / 1.5f),
			m_SubmenuBarTextSize, m_SubmenuBarFont,
			m_SubmenuBarTextColor,
			false, true);
		/*DrawRightText(
			&rightText[0],
			m_PosX + (m_Width / m_SubmenuBarPadding),
			m_DrawBaseY + (m_SubmenuBarHeight / 2.f) - (GetTextHeight(m_SubmenuBarFont, m_SubmenuBarTextSize) / 1.5f),
			m_SubmenuBarTextSize, m_SubmenuBarFont,
			m_SubmenuBarTextColor,
			false, true);
			*/
		m_DrawBaseY += m_SubmenuBarHeight;
	}

	void UIManager::DrawOption(AbstractOption* opt, bool selected)
	{
		DrawRect(
			m_PosX,
			m_DrawBaseY + (m_OptionHeight / 2.f),
			m_Width,
			m_OptionHeight,
			selected ? m_OptionSelectedBackgroundColor : m_OptionUnselectedBackgroundColor);
		DrawLeftText(
			opt->GetLeftText(),
			m_PosX - (m_Width / m_OptionPadding),
			m_DrawBaseY + (m_OptionHeight / 2.f) - (GetTextHeight(m_OptionFont, m_OptionTextSize) / 1.5f),
			m_OptionTextSize,
			m_OptionFont,
			selected ? m_OptionSelectedTextColor : m_OptionUnselectedTextColor,
			false, false);
		DrawRightText(
			opt->GetRightText(),
			m_PosX + (m_Width / m_OptionPadding),
			m_DrawBaseY + (m_OptionHeight / 2.f) - (GetTextHeight(m_OptionFont, m_OptionTextSize) / 1.5f),
			m_OptionTextSize,
			m_OptionFont,
			selected ? m_OptionSelectedTextColor : m_OptionUnselectedTextColor,
			false, false);

		if (opt->GetFlag(OptionFlag::Enterable))
		{
			DrawRightText(
				">",
				m_PosX + (m_Width / m_OptionPadding),
				m_DrawBaseY + (m_OptionHeight / 2.f) - (GetTextHeight(Font::Monospace, m_OptionTextSize) / 1.725f),
				m_OptionTextSize,
				Font::Monospace,
				selected ? m_OptionSelectedTextColor : m_OptionUnselectedTextColor,
				false, false);
		}

		m_DrawBaseY += m_OptionHeight;
	}

	void UIManager::DrawFooter(AbstractSubmenu* sub)
	{
		DrawRect(
			m_PosX,
			m_DrawBaseY + (m_FooterHeight / 2.f),
			m_Width,
			m_FooterHeight,
			m_FooterBackgroundColor);
		
		float size = m_FooterSpriteSize;
		/*float rotation = 0.f;
		const char* texture = "shop_arrows_upanddown";

		if (!m_SubmenuStack.empty())
		{
			auto sub = m_SubmenuStack.top();
			if (sub->GetSelectedOption() == 0)
			{
				rotation = 90.f;
				texture = "arrowright";
				size *= 0.8f;
			}
			else if (sub->GetSelectedOption() + 1 == sub->GetNumOptions())
			{
				rotation = 270.f;
				texture = "arrowright";
				size *= 0.8f;
			}
		}
		*/

		auto sizee = GetSpriteScale(m_FooterSpriteSize);


		DrawSprite(
			"FrontierTextures",
			"LogoW",
			m_PosX,
			m_DrawBaseY + (m_FooterHeight / 2.f),
			sizee.x,
			sizee.y,
			m_FooterSpriteColor,
			0);

		char rightText[32] = {};
		std::snprintf(rightText, sizeof(rightText) - 1, "%zu / %zu", sub->GetSelectedOption() + 1, sub->GetNumOptions());
		DrawRightText(
			&rightText[0],
			m_PosX + (m_Width / m_SubmenuBarPadding),
			m_DrawBaseY + (m_SubmenuBarHeight / 2.f) - (GetTextHeight(m_SubmenuBarFont, m_SubmenuBarTextSize) / 1.5f),
			m_SubmenuBarTextSize, m_SubmenuBarFont,
			m_SubmenuBarTextColor,
			false, true);

		m_DrawBaseY += m_FooterHeight;
	}

	void UIManager::DrawDescription()
	{
		const char* description{};

		if (!m_SubmenuStack.empty())
		{
			auto sub = m_SubmenuStack.top();
			if (sub->GetNumOptions())
			{
				if (auto opt = sub->GetOption(sub->GetSelectedOption()))
				{
					description = opt->GetDescription();
				}
			}
		}

		if (!description || !*description)
			return;

		m_DrawBaseY += m_DescriptionHeightPadding;

		

		/*Sprite
		auto spriteSize = GetSpriteScale(m_DescriptionSpriteSize);
		DrawSprite(
			"shared", "info_icon_32",
			m_PosX - (m_Width / m_DescriptionPadding) + (spriteSize.x / 2.f),
			m_DrawBaseY + (m_DescriptionHeight / 2.f),
			spriteSize.x,
			spriteSize.y,
			m_DescriptionSpriteColor,
			0.f
		);
		*/

		/*Background default
		DrawRect(
			m_PosX,
			m_DrawBaseY + (m_DescriptionHeight / 2.f),
			m_Width,
			m_DescriptionHeight,
			m_DescriptionBackgroundColor);
			*/

		float RectWidth = m_Width * 1.2;
		float RectHeight = m_DescriptionHeight * 2;

		float posX = 0.5f;
		float posY = 0.925f;

		DrawRect(
			posX,
			posY,
			RectWidth,
			RectHeight,
			m_FooterBackgroundColor);
		DrawCenteredText(
			description,
			0.5f,
			0.915f ,
			m_DescriptionTextSize,
			m_DescriptionFont,
			m_DescriptionTextColor,
			false, false
		);

		

		//TopL
		Vector2 TopL = { posX - (0.5f * RectWidth), posY - (0.5f * RectHeight) };

		//TopR
		Vector2 TopR = { posX + (0.5f * RectWidth), posY - (0.5f * RectHeight) };


		//BottomL
		Vector2 BotL = { posX - (0.5f * RectWidth), posY + (0.5f * RectHeight) };

		//BottomR
		Vector2 BotR = { posX + (0.5f * RectWidth), posY + (0.5f * RectHeight) };


		float thickness = 0.001f;

		//Top
		DrawRect(
			(TopL.x + TopR.x)/2,
			TopL.y + (thickness*2),
			TopR.x - TopL.x -(thickness*2),
			thickness,
			m_OptionSelectedBackgroundColor
		);

		//Bottom
		DrawRect(
			(BotL.x + BotR.x) / 2,
			BotL.y - (thickness * 2),
			BotR.x - BotL.x - (thickness * 2),
			thickness,
			m_OptionSelectedBackgroundColor
		);


		/*Text
		DrawLeftText(
			description,
			m_PosX - (m_Width / m_DescriptionPadding), //+ (spriteSize.x * 1.15f),
			m_DrawBaseY + (m_DescriptionHeight / 2.f) - (GetTextHeight(m_DescriptionFont, m_DescriptionTextSize) / 1.5f),
			m_DescriptionTextSize,
			m_DescriptionFont,
			m_DescriptionTextColor,
			false, false
		);
		*/

		m_DrawBaseY += m_DescriptionHeight;
	}

	void UIManager::DrawRect(float x, float y, float width, float height, Color color)
	{
		GRAPHICS::DRAW_RECT(x, y, width, height, color.r, color.g, color.b, color.a, 0);
	}

	void UIManager::DrawSprite(const char* dict, const char* texture, float x, float y, float width, float height, Color color, float rotation)
	{
		if (GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED(dict))
		{
			GRAPHICS::DRAW_SPRITE(dict, texture, x, y, width, height, rotation, color.r, color.g, color.b, color.a, 0);
		}
		else
		{
			GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT(dict, false);
		}
	}

	void UIManager::DrawLeftText(const char* text, float x, float y, float size, Font font, Color color, bool outline, bool shadow)
	{
		HUD::SET_TEXT_SCALE(size, size);
		HUD::SET_TEXT_FONT(static_cast<int>(font));
		HUD::SET_TEXT_COLOUR(color.r, color.g, color.b, color.a);
		if (outline)
			HUD::SET_TEXT_OUTLINE();
		if (shadow)
			HUD::SET_TEXT_DROP_SHADOW();

		HUD::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
		HUD::END_TEXT_COMMAND_DISPLAY_TEXT(x, y, 0);
	}

	void UIManager::DrawCenteredText(const char* text, float x, float y, float size, Font font, Color color, bool outline, bool shadow)
	{
		HUD::SET_TEXT_CENTRE(true);
		DrawLeftText(text, x, y, size, font, color, outline, shadow);
	}

	void UIManager::DrawRightText(const char* text, float x, float y, float size, Font font, Color color, bool outline, bool shadow)
	{
		HUD::SET_TEXT_WRAP(0.f, x);
		HUD::SET_TEXT_RIGHT_JUSTIFY(true);
		DrawLeftText(text, x, y, size, font, color, outline, shadow);
	}

	float UIManager::GetTextHeight(Font font, float size)
	{
		return HUD::_GET_TEXT_SCALE_HEIGHT(size, static_cast<int>(font));
	}

	Vector2 UIManager::GetSpriteScale(float size)
	{
		int x;
		int y;
		GRAPHICS::_GET_ACTIVE_SCREEN_RESOLUTION(&x, &y);

		return { (static_cast<float>(y) / static_cast<float>(x)) * size, size };
	}

	bool UIManager::IsMouseLocked()
	{
		return m_Opened && m_MouseLocked;
	}

	Rectangle UIManager::GetMenuRect()
	{
		float height = m_HeaderHeight;
		height += m_SubmenuBarHeight;

		if (!m_SubmenuStack.empty())
		{
			height += m_OptionHeight * std::min(m_SubmenuStack.top()->GetNumOptions(), m_OptionsPerPage);
		}

		height += m_FooterHeight;

		return
		{
			{ m_PosX + (m_Width / 2.f), m_PosY + (height / 2.f) },
			{ m_Width, height }
		};
	}

	Vector2 UIManager::GetMousePos()
	{
		POINT point;
		GetCursorPos(&point);

		return Vector2{ static_cast<float>(point.x), static_cast<float>(point.y) };
	}

	void UIManager::SaveColors() {
		std::string AppDataPath = getenv("APPDATA");
		std::string MenuFolderPath = AppDataPath + "\\Frontier\\";
		Ini* ColorIni = new Ini(MenuFolderPath + "Colors.ini");

		ColorIni->WriteFloat(m_PosX, "General", "PosX");
		ColorIni->WriteFloat(m_PosY, "General", "PosY");
		ColorIni->WriteFloat(m_Width, "General", "Width");

		ColorIni->WriteInt(m_OpenDelay, "Input", "ODelay");
		ColorIni->WriteInt(m_BackDelay, "Input", "BDelay");
		ColorIni->WriteInt(m_VerticalDelay, "Input", "VDelay");
		ColorIni->WriteInt(m_HorizontalDelay, "Input", "HDelay");

		ColorIni->WriteFloat(m_HeaderHeight, "Header", "Hei");
		if (m_HeaderType == HeaderType::Gradient)
		ColorIni->WriteInt(1, "Header", "Anim");
		else ColorIni->WriteInt(0, "Header", "Anim");

		ColorIni->WriteFloat(m_SubmenuBarHeight, "SubBar", "SHei");
		ColorIni->WriteFloat(m_SubmenuBarTextSize, "SubBar", "txtSi");
		ColorIni->WriteFloat(m_SubmenuBarPadding, "SubBar", "txtPad");
		ColorIni->WriteInt(m_SubmenuBarBackgroundColor.r, "SubBar", "BackR");
		ColorIni->WriteInt(m_SubmenuBarBackgroundColor.g, "SubBar", "BackG");
		ColorIni->WriteInt(m_SubmenuBarBackgroundColor.b, "SubBar", "BackB");
		ColorIni->WriteInt(m_SubmenuBarBackgroundColor.a, "SubBar", "BackA");
		ColorIni->WriteInt(m_SubmenuBarTextColor.r, "SubBar", "TxtR");
		ColorIni->WriteInt(m_SubmenuBarTextColor.g, "SubBar", "TxtG");
		ColorIni->WriteInt(m_SubmenuBarTextColor.b, "SubBar", "TxtB");
		ColorIni->WriteInt(m_SubmenuBarTextColor.a, "SubBar", "TxtA");

		ColorIni->WriteFloat(m_OptionHeight, "Options", "Hei");
		ColorIni->WriteFloat(m_OptionTextSize, "Options", "txtSz");
		ColorIni->WriteFloat(m_OptionPadding, "Options", "optPad");


		ColorIni->WriteInt(m_OptionSelectedTextColor.r, "Options", "STxtR");
		ColorIni->WriteInt(m_OptionSelectedTextColor.g, "Options", "STxtG");
		ColorIni->WriteInt(m_OptionSelectedTextColor.b, "Options", "STxtB");
		ColorIni->WriteInt(m_OptionSelectedTextColor.a, "Options", "STxtA");

		ColorIni->WriteInt(m_OptionUnselectedTextColor.r, "Options", "UTxtR");
		ColorIni->WriteInt(m_OptionUnselectedTextColor.g, "Options", "UTxtG");
		ColorIni->WriteInt(m_OptionUnselectedTextColor.b, "Options", "UTxtB");
		ColorIni->WriteInt(m_OptionUnselectedTextColor.a, "Options", "UTxtA");

		ColorIni->WriteInt(m_OptionSelectedBackgroundColor.r, "Options", "SBckR");
		ColorIni->WriteInt(m_OptionSelectedBackgroundColor.g, "Options", "SBckG");
		ColorIni->WriteInt(m_OptionSelectedBackgroundColor.b, "Options", "SBckB");
		ColorIni->WriteInt(m_OptionSelectedBackgroundColor.a, "Options", "SBckA");

		ColorIni->WriteInt(m_OptionUnselectedBackgroundColor.r, "Options", "UBckR");
		ColorIni->WriteInt(m_OptionUnselectedBackgroundColor.g, "Options", "UBckG");
		ColorIni->WriteInt(m_OptionUnselectedBackgroundColor.b, "Options", "UBckB");
		ColorIni->WriteInt(m_OptionUnselectedBackgroundColor.a, "Options", "UBckA");
		
		ColorIni->WriteFloat(m_FooterHeight, "Footer", "FHei");
		ColorIni->WriteFloat(m_FooterSpriteSize, "Footer", "FSprS");
		ColorIni->WriteInt(m_FooterBackgroundColor.r, "Options", "FBckR");
		ColorIni->WriteInt(m_FooterBackgroundColor.g, "Options", "FBckG");
		ColorIni->WriteInt(m_FooterBackgroundColor.b, "Options", "FBckB");
		ColorIni->WriteInt(m_FooterBackgroundColor.a, "Options", "FBckA");
		ColorIni->WriteInt(m_FooterSpriteColor.r, "Options", "FSprR");
		ColorIni->WriteInt(m_FooterSpriteColor.g, "Options", "FSprG");
		ColorIni->WriteInt(m_FooterSpriteColor.b, "Options", "FSprB");
		ColorIni->WriteInt(m_FooterSpriteColor.a, "Options", "FSprA");

		ColorIni->WriteFloat(m_DescriptionHeightPadding, "Descrip", "DescHeiPad");
		ColorIni->WriteFloat(m_DescriptionHeight, "Descrip", "Deschei");
		ColorIni->WriteFloat(m_DescriptionTextSize, "Descrip", "DescFSize");
		ColorIni->WriteFloat(m_DescriptionSpriteSize, "Descrip", "DescSSize");
		ColorIni->WriteFloat(m_DescriptionPadding, "Descrip", "DescPad");

		ColorIni->WriteInt(m_DescriptionBackgroundColor.r, "Descrip", "DeBckR");
		ColorIni->WriteInt(m_DescriptionBackgroundColor.g, "Descrip", "DeBckG");
		ColorIni->WriteInt(m_DescriptionBackgroundColor.b, "Descrip", "DeBckB");
		ColorIni->WriteInt(m_DescriptionBackgroundColor.a, "Descrip", "DeBckA");

		ColorIni->WriteInt(m_DescriptionTextColor.r, "Descrip", "DeTxtR");
		ColorIni->WriteInt(m_DescriptionTextColor.g, "Descrip", "DeTxtG");
		ColorIni->WriteInt(m_DescriptionTextColor.b, "Descrip", "DeTxtB");
		ColorIni->WriteInt(m_DescriptionTextColor.a, "Descrip", "DeTxtA");

		ColorIni->WriteInt(m_DescriptionSpriteColor.r, "Descrip", "DeSprR");
		ColorIni->WriteInt(m_DescriptionSpriteColor.g, "Descrip", "DeSprG");
		ColorIni->WriteInt(m_DescriptionSpriteColor.b, "Descrip", "DeSprB");
		ColorIni->WriteInt(m_DescriptionSpriteColor.a, "Descrip", "DeSprA");



		ColorIni->WriteFloat(ScrollerPadding , "ScrollB", "Padd");
		ColorIni->WriteBool(DrawScroller, "ScrollB", "DrwScrl");
		ColorIni->WriteBool(isLeftScroller, "ScrollB", "isLftScrl");



		g_Logger->Info("Saved Menu Apearance!");
	}

	void UIManager::LoadColors() {
		std::string AppDataPath = getenv("APPDATA");
		std::string MenuFolderPath = AppDataPath + "\\Frontier\\";
		if (ColorIniExist((MenuFolderPath + "Colors.ini").c_str())) {
			Ini* ColorIni = new Ini(MenuFolderPath + "Colors.ini");

			m_PosX = ColorIni->GetFloat("General", "PosX");
			m_PosY = ColorIni->GetFloat("General", "PosY");
			m_Width = ColorIni->GetFloat("General", "Width");


			m_OpenDelay = ColorIni->GetInt("Input", "ODelay");
			m_BackDelay = ColorIni->GetInt("Input", "BDelay");
			m_VerticalDelay = ColorIni->GetInt("Input", "VDelay");
			m_HorizontalDelay = ColorIni->GetInt("Input", "HDelay");

			m_HeaderHeight = ColorIni->GetFloat("Header", "Hei");

			int Anim = ColorIni->GetInt("Header", "Anim");
			if (Anim == 1)
				m_HeaderType = HeaderType::Gradient;
			else m_HeaderType = HeaderType::Static;

			m_SubmenuBarHeight = ColorIni->GetFloat("SubBar", "SHei");
			m_SubmenuBarTextSize = ColorIni->GetFloat("SubBar", "txtSi");
			m_SubmenuBarPadding = ColorIni->GetFloat("SubBar", "txtPad");

			m_SubmenuBarBackgroundColor = { (uint8_t)ColorIni->GetInt("SubBar", "BackR"), (uint8_t)ColorIni->GetInt("SubBar", "BackG") , (uint8_t)ColorIni->GetInt("SubBar", "BackB") , (uint8_t)ColorIni->GetInt("SubBar", "BackA") };
			m_SubmenuBarTextColor = { (uint8_t)ColorIni->GetInt("SubBar", "TxtR"), (uint8_t)ColorIni->GetInt("SubBar", "TxtG") , (uint8_t)ColorIni->GetInt("SubBar", "TxtB") , (uint8_t)ColorIni->GetInt("SubBar", "TxtA") };


			m_OptionHeight = ColorIni->GetFloat("Options", "Hei");
			m_OptionTextSize = ColorIni->GetFloat("Options", "txtSz");
			m_OptionPadding = ColorIni->GetFloat("Options", "optPad");

			m_OptionSelectedTextColor = { (uint8_t)ColorIni->GetInt("Options", "STxtR"), (uint8_t)ColorIni->GetInt("Options", "STxtG") , (uint8_t)ColorIni->GetInt("Options", "STxtB") , (uint8_t)ColorIni->GetInt("Options", "STxtA") };
			m_OptionUnselectedTextColor = { (uint8_t)ColorIni->GetInt("Options", "UTxtR"), (uint8_t)ColorIni->GetInt("Options", "UTxtG") , (uint8_t)ColorIni->GetInt("Options", "UTxtB") , (uint8_t)ColorIni->GetInt("Options", "UTxtA") };
			m_OptionSelectedBackgroundColor = { (uint8_t)ColorIni->GetInt("Options", "SBckR"), (uint8_t)ColorIni->GetInt("Options", "SBckG") , (uint8_t)ColorIni->GetInt("Options", "SBckB") , (uint8_t)ColorIni->GetInt("Options", "SBckA") };
			m_OptionUnselectedBackgroundColor = { (uint8_t)ColorIni->GetInt("Options", "UBckR"), (uint8_t)ColorIni->GetInt("Options", "UBckG") , (uint8_t)ColorIni->GetInt("Options", "UBckB") , (uint8_t)ColorIni->GetInt("Options", "UBckA") };


			m_FooterHeight = ColorIni->GetFloat("Footer", "FHei");
			m_FooterSpriteSize = ColorIni->GetFloat("Footer", "FSprS");

			m_FooterBackgroundColor = { (uint8_t)ColorIni->GetInt("Options", "FBckR"), (uint8_t)ColorIni->GetInt("Options", "FBckG") , (uint8_t)ColorIni->GetInt("Options", "FBckB") , (uint8_t)ColorIni->GetInt("Options", "FBckA") };
			m_FooterSpriteColor = { (uint8_t)ColorIni->GetInt("Options", "FSprR"), (uint8_t)ColorIni->GetInt("Options", "FSprG") ,(uint8_t)ColorIni->GetInt("Options", "FSprB") , (uint8_t)ColorIni->GetInt("Options", "FSprA") };

			m_DescriptionHeightPadding = ColorIni->GetFloat("Descrip", "DescHeiPad");
			m_DescriptionHeight = ColorIni->GetFloat("Descrip", "Deschei");
			m_DescriptionTextSize = ColorIni->GetFloat("Descrip", "DescFSize");
			m_DescriptionSpriteSize = ColorIni->GetFloat("Descrip", "DescSSize");
			m_DescriptionPadding = ColorIni->GetFloat("Descrip", "DescPad");

			m_DescriptionBackgroundColor = { (uint8_t)ColorIni->GetInt("Descrip", "DeBckR"), (uint8_t)ColorIni->GetInt("Descrip", "DeBckG") , (uint8_t)ColorIni->GetInt("Descrip", "DeBckB") , (uint8_t)ColorIni->GetInt("Descrip", "DeBckA") };
			m_DescriptionTextColor = { (uint8_t)ColorIni->GetInt("Descrip", "DeTxtR"), (uint8_t)ColorIni->GetInt("Descrip", "DeTxtG") , (uint8_t)ColorIni->GetInt("Descrip", "DeTxtB") , (uint8_t)ColorIni->GetInt("Descrip", "DeTxtA") };
			m_DescriptionSpriteColor = { (uint8_t)ColorIni->GetInt("Descrip", "DeSprR"), (uint8_t)ColorIni->GetInt("Descrip", "DeSprG") , (uint8_t)ColorIni->GetInt("Descrip", "DeSprB") , (uint8_t)ColorIni->GetInt("Descrip", "DeSprA") };

		
			ScrollerPadding = ColorIni->GetFloat("ScrollB", "Padd");
			DrawScroller = ColorIni->GetInt("ScrollB", "DrwScrl");
			isLeftScroller = ColorIni->GetInt("ScrollB", "isLftScrl");

			g_Logger->Info("Loaded Custom Menu Appearance!");
		}
		else {
			DefaultAppearance();
			g_Logger->Info("Loaded Default Menu Appearance!");
		}

	}

	void UIManager::DefaultAppearance() {
		m_PosX = 0.8f;
		m_PosY = 0.1f;
		m_Width = 0.21f;
		
		// Input
		m_OpenDelay = 200;
		m_BackDelay = 300;
		m_EnterDelay = 300;
		m_VerticalDelay = 120;
		m_HorizontalDelay = 120;

		// Header
		m_HeaderHeight = 0.1f;
		m_HeaderText = false;
		m_HeaderTextSize = 1.f;

		m_HeaderBackgroundColor={ 2, 130, 53, 255 };
		m_HeaderTextColor={ 255, 255, 255, 255 };
		m_HeaderType = HeaderType::Gradient;

		// Submenu bar
		m_SubmenuBarHeight = 0.033f;
		m_SubmenuBarTextSize = 0.37f;
		m_SubmenuBarFont = Font::ChaletComprimeCologne;
		m_SubmenuBarPadding = 2.1f;
		m_SubmenuBarBackgroundColor={ 37, 37, 37, 255 };
		m_SubmenuBarTextColor={ 255, 255, 255, 255 };

		// Options
		m_OptionHeight = 0.038f;
		m_OptionTextSize = 0.33f;
		m_OptionFont = Font::ChaletLondon;
		m_OptionPadding = 2.1f;
		m_OptionSelectedTextColor={ 255, 255, 255, 255 };
		m_OptionUnselectedTextColor={ 255, 255, 255, 255 };
		m_OptionSelectedBackgroundColor={ 0, 150, 255, 255 };
		m_OptionUnselectedBackgroundColor={ 23, 23, 23, 210 };

		// Footer
		m_FooterHeight = 0.032f;
		m_FooterSpriteSize = 0.042f;
		m_FooterBackgroundColor={ 37, 37, 37, 255 };
		m_FooterSpriteColor={ 0, 150, 255, 255 };

		// Description
		m_DescriptionHeightPadding = 0.01f;
		m_DescriptionHeight = 0.033f;
		m_DescriptionTextSize = 0.28f;
		m_DescriptionBackgroundColor={ 0, 0, 0, 120 };
		m_DescriptionTextColor={ 255, 255, 255, 255 };
		m_DescriptionSpriteSize = 0.025f;
		m_DescriptionPadding = 2.1f;
		m_DescriptionSpriteColor={ 255, 255, 255, 255 };


		//Scroller
		isLeftScroller = false;
		DrawScroller = true;
		ScrollerPadding = 0.01f;

	}

	bool UIManager::ColorIniExist(const char* path)
	{

		struct stat buffer;
		return (stat(path, &buffer) == 0);

	}
}





using namespace Memory;
namespace Big {

	//Custom Textures
	static Big::fpFileRegister file_register = (Big::fpFileRegister)(pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 50 48 8B EA 4C 8B FA 48 8B D9 4D 85 C9").count(1).get(0).get<decltype(file_register)>());
	bool FileExists(const std::string& fileName)
	{
		struct stat buffer;
		return (stat(fileName.c_str(), &buffer) == 0);
	}
	void YTD()
	{
		std::string AppDataPath = getenv("APPDATA");
		std::string MenuFolderPath = AppDataPath + "\\Frontier\\";
		std::string name = "FrontierTextures.ytd";

		const std::string fullPath = MenuFolderPath + name;

		int textureID;
		if (FileExists(fullPath))
			file_register(&textureID, fullPath.c_str(), true, name.c_str(), false);
		g_Logger->Info("YTD LOADED");
	}


}
#include "game.hpp"
#include "SDLWrapper.hpp"

template <typename T>
gobl::vec2i DrawCards(std::vector<T>& cards, int& holdIndex, Game::HoldingType& holding, gobl::vec2i pos)
{
	// Ensure T is derived from Card
	static_assert(std::is_base_of<Card, T>::value, "T must be derived from Card");

	const int SPACING = 15 * 4;
	for (int i = 0; i < cards.size(); i++)
	{
		auto item = cards.at(i);
		bool mouseOver = item.mouseOver(pos);

		if (holdIndex == i && holding == (Game::HoldingType)cards.at(0).type)
		{
			item.Draw(SDLWrapper::getMousePos() + gobl::vec2<int>{ 0, -16 });

			if (SDLWrapper::getMouse().y > pos.y - 20) pos.x += (SPACING * 2) + 10;
		}
		else
		{
			if (mouseOver) pos.y -= 10;

			item.Draw(pos);

			if (mouseOver)
			{
				if (SDLWrapper::getKeyboard().bHeld(SDLK_LALT))
				{
					item.DrawZoomed({ 0, 0 });
				}
				else if (SDLWrapper::getMouse().bHeld(0) && holdIndex == -1)
				{
					holdIndex = i;
					holding = (Game::HoldingType)item.type;
				}
				else
				{
					SDLWrapper::DrawString("hold " + std::string(SDL_GetKeyName(SDLK_LALT)) + " to magnify", { pos.x, pos.y - 16 }, sdl::BLACK, 12);
					// SDLWrapper::DrawString("type: " + std::to_string(item.type) + " index: " + std::to_string(i), { pos.x, pos.y - 32 }, sdl::BLACK, 12);
				}

				pos.y += 20;
				pos.x += SPACING + 10;
			}

			pos.x += SPACING;
			pos.y += i % 2 == 0 ? 2 : -2;
		}
	}

	return pos;
}

void Game::DisplayAccusing()
{
	static bool foundWhat = false;
	static int accusing = -1;
	int hoverQ = -1;

	for (int i = 0; i < questions.size(); i++)
	{
		questions.at(i).Draw();
		if (questions.at(i).mouseOver()) hoverQ = i;

		if (questions.at(i).text == "Why?" && accusing != -1) questions.at(i).answer = (gameData->suspects.at(accusing).foundMotive) ? gameData->suspects.at(accusing).GetMotive() : "???";
		if (questions.at(i).text == "Where?") questions.at(i).answer = gameData->mapView->GetMurderRoom();
	}

	gobl::vec2i pos = DrawCards(gameData->weapons, holdIndex, holding, { 5, SDLWrapper::getScreenHeight() - (Card::CARD_RECT.y + 3) });
	DrawCards(gameData->suspects, holdIndex, holding, pos);
	if (SDLWrapper::getMouse().bRelease(0))
	{
		if (hoverQ > -1)
		{
			if (questions.at(hoverQ).text == "Who?" && holding == SUSPECT)
			{
				questions.at(hoverQ).answer = gameData->suspects.at(holdIndex).name;
				accusing = holdIndex;
			}
			if (questions.at(hoverQ).text == "What?" && holding == WEAPON)
			{
				questions.at(hoverQ).answer = gameData->weapons.at(holdIndex).name;
				if (questions.at(hoverQ).answer == gameData->weapons.at(gameData->weapon).name) foundWhat = true;
				else foundWhat = false;
			}
		}

		holdIndex = -1;
		holding = NONE;
	}

	static Button btn = { .onClick = [&]() {
		if ((gameData->killer == accusing) && foundWhat) state = Win;
		else
		{
			accusing = -1;
			for (auto& q : questions) q.answer = "";
		}
	}, .text = "Accuse", .pos = questions.back().pos + gobl::vec2<int>{ 0, 50} };
	btn.Draw();
}

void Game::DisplayKiller(bool foundKiller)
{
	gobl::vec2<int> killerPos = { 100, 100 };
	gobl::vec2<int> weaponPos = { 100, 120 + gameData->suspectSprite.height };

	int score = 0;

	if (foundKiller)
	{
		gameData->suspects.at(gameData->killer).Draw(killerPos);
		SDLWrapper::DrawString("The killer was " + gameData->suspects.at(gameData->killer).name, killerPos + gobl::vec2<int>{ 0, gameData->suspectSprite.height }, sdl::BLACK);

		gameData->weapons.at(gameData->weapon).Draw(weaponPos);
		SDLWrapper::DrawString("with the " + gameData->weapons.at(gameData->weapon).name, weaponPos + gobl::vec2<int>{ 0, gameData->weaponSprite.height }, sdl::BLACK);
		score += 50;
	}
	else SDLWrapper::DrawString("The killer wasn't caught...", killerPos + gobl::vec2<int>{ 0, gameData->suspectSprite.height }, sdl::BLACK);

	if (gameData->mapView->GetMurderRoom() != "???")
	{
		SDLWrapper::DrawString("they struck in the " + gameData->mapView->GetMurderRoom(), weaponPos + gobl::vec2<int>{ 0, gameData->weaponSprite.height + 24 }, sdl::BLACK);
		score += 25;
	}
	else SDLWrapper::DrawString("We may never know where...", weaponPos + gobl::vec2<int>{ 0, gameData->weaponSprite.height + 24 }, sdl::BLACK);

	if (gameData->suspects.at(gameData->killer).foundMotive)
	{
		SDLWrapper::DrawString("their motive was " + gameData->suspects.at(gameData->killer).GetMotive(), weaponPos + gobl::vec2<int>{ 0, gameData->weaponSprite.height + 36 }, sdl::BLACK);
		score += 25;
	}
	else SDLWrapper::DrawString("We may never know why...", weaponPos + gobl::vec2<int>{ 0, gameData->weaponSprite.height + 36 }, sdl::BLACK);

	std::string grade = "F";
	if (score > 80) grade = "A";
	else if (score > 70) grade = "C";
	else if (score > 0) grade = "D";

	SDLWrapper::DrawString("CASE GRADE: " + grade, gobl::vec2<int>{ SDLWrapper::getScreenWidth() - 220, SDLWrapper::getScreenHeight() - 36 }, sdl::BLACK);
}

void Game::DisplayInterview(float deltaTime)
{
	auto& curScene = gameData->scenes.at("conversation"); // FIXME: Use a dynamic name instead of "conversation"
	gobl::vec2<int> speachBubblePos = gobl::vec2<int>{ 10, SDLWrapper::getScreenHeight() >> 1 };
	gobl::vec2<int> suspectPos = gobl::vec2<int>{ 100, speachBubblePos.y - gameData->suspectSprite.height };

	gameData->suspects.at(interviewing).Draw(suspectPos);
	speachBubblePos.y += 25;
	SDLWrapper::DrawString(curScene.speakerState, speachBubblePos, sdl::WHITE);
	speachBubblePos.y += 25;

	static QuestionObject responseBox = { .text = "Response ", .pos = gobl::vec2<int>{speachBubblePos.x + 100, speachBubblePos.y + 25} };
	responseBox.Draw();

	if (responseBox.answer.size() < 2)
	{
		DrawCards(curScene.response, holdIndex, holding, { 5, SDLWrapper::getScreenHeight() - (Card::CARD_RECT.y + 3) });

		if (SDLWrapper::getMouse().bRelease(0) && holdIndex != -1)
		{
			if (responseBox.mouseOver()) responseBox.answer = gameData->scenes.at("conversation").response.at(holdIndex).name;

			curScene.outcomeState = holdIndex;

			holdIndex = -1;
		}
	}
	else
	{
		// Don't handle outcomes until AFTER we've determined if there's a second step
		if (curScene.secondStep.at(curScene.outcomeState).size() > 1 && curScene.finalState == -1)
		{
			if (curScene.finalState == -1)
			{
				curScene.speakerState = "Yes?";
				if (curScene.secondStep.at(curScene.outcomeState) == "weapon")
				{
					DrawCards(gameData->weapons, holdIndex, holding, { 5, SDLWrapper::getScreenHeight() - (Card::CARD_RECT.y + 3) });

					if (SDLWrapper::getMouse().bRelease(0) && holdIndex != -1)
					{
						if (responseBox.mouseOver()) responseBox.answer = gameData->weapons.at(holdIndex).name;
						curScene.finalState = holdIndex;

						holdIndex = -1;
					}
				}
				else if (curScene.secondStep.at(curScene.outcomeState) == "suspect")
				{
					std::vector<Suspect> newSuspects{};
					for (int i = 0; i < gameData->suspects.size(); i++)
					{
						if (i == interviewing) continue;
						newSuspects.push_back(gameData->suspects.at(i));
					}

					DrawCards(newSuspects, holdIndex, holding, { 5, SDLWrapper::getScreenHeight() - (Card::CARD_RECT.y + 3) });

					if (SDLWrapper::getMouse().bRelease(0) && holdIndex != -1)
					{
						if (responseBox.mouseOver()) responseBox.answer = gameData->weapons.at(holdIndex).name;
						curScene.finalState = holdIndex;

						holdIndex = -1;
					}
				}
			}
		}
		else
		{
			const auto& outcomes = curScene.outcomes.at(curScene.outcomeState);
			for (int o = 0; o < outcomes.size(); o++)
			{
				if (outcomes.at(o) == "motive" && interviewing != gameData->killer)
				{
					curScene.speakerState = "I heard that they're motive would be " + gameData->suspects.at(curScene.finalState).GetMotive();
					gameData->suspects.at(curScene.finalState).foundMotive = true;
				}

				if (outcomes.at(o) == "suspectMisdirect" && interviewing == gameData->killer)
				{
					curScene.speakerState = "I think I saw the butler...";
				}

				if (outcomes.at(o) == "weaponMisdirect" && interviewing == gameData->killer)
				{
					if (gameData->weapon != curScene.finalState) curScene.speakerState = "I'm pretty sure I saw someone with a " + gameData->weapons.at(curScene.finalState).name + " impression on their head...";
					else curScene.speakerState = "I saw a shadowy figure walking with the " + gameData->weapons.at(rand() % gameData->weapons.size()).name;
				}

				if (outcomes.at(o) == "weapon" && interviewing != gameData->killer)
				{
					if (gameData->weapon == curScene.finalState) curScene.speakerState = "I mean it's got blood all over it.";
					else curScene.speakerState = "I've never seen that before in my life";
				}

				if (outcomes.at(o) == "accuse")
				{
					curScene.speakerState = curScene.speakerInitState;
					state = Accusing;
				}

				if (outcomes.at(o) == "end")
				{
					curScene.speakerState = curScene.speakerInitState;
					state = Investigating;
				}
			}

			curScene.finalState = -1;
			responseBox.answer = "";
		}
	}
}

void Game::DisplayIntroduction(float deltaTime)
{
	//SDLWrapper::DrawSprite(gameData->introScene.room);
	gameData->mapView->DrawRoom(gameData->introScene.room);

	const int FONT_SIZE = 32;
	gobl::vec2i introLinePos = { 100, SDLWrapper::getScreenHeight() >> 1 };
	SDLWrapper::DrawString(gameData->introScene.line, introLinePos + gobl::vec2i{ -2, 2 }, sdl::DARK_GREY, FONT_SIZE); // Shadow
	SDLWrapper::DrawString(gameData->introScene.line, introLinePos, sdl::WHITE, FONT_SIZE);

	static QuestionObject continueFactor = { .text = "Response ", .pos = {100, (SDLWrapper::getScreenHeight() >> 1) + 50} };
	continueFactor.Draw();

	if (continueFactor.answer.size() < 2)
	{
		DrawCards(gameData->introScene.response, holdIndex, holding, { 5, SDLWrapper::getScreenHeight() - (Card::CARD_RECT.y + 3) });

		if (SDLWrapper::getMouse().bRelease(0) && holdIndex != -1)
		{
			if (continueFactor.mouseOver()) continueFactor.answer = gameData->introScene.response.at(holdIndex).name;
			holdIndex = -1;
		}
	}
	else
	{
		const float MAX_TIME = 5.0f;
		static float time = 0.0f;
		time += deltaTime;
		SDLWrapper::DrawString("Arriving to crime scene in " + std::to_string(int(ceil(MAX_TIME - time))) + "...", gobl::vec2<int>{ 10, SDLWrapper::getScreenHeight() - 16 }, sdl::BLACK);
		if (time >= MAX_TIME) state = Investigating;
	}
}

void Game::DisplayRoomEditor(float deltaTime)
{
	static int curEdit = -1;
	SDLWrapper::SetClear(SDL_Color(0, 0, 0, 255));

	auto DrawPanel = [&]()
		{
			SDLWrapper::DrawString("Select room:");

			int y = 16, x = 50;
			for (int i = 0; i < gameData->rooms.size(); i++)
			{
				Button btn{ .onClick = [&] {
						curEdit = i;
					}, .text = gameData->rooms.at(i).name, .pos = {x, y} };

				btn.Draw();
				y += 20;
			}
		};

	if (curEdit == -1) DrawPanel();
	else
	{
		SDL_Color textCol = sdl::WHITE;
		if (gameData->rooms.at(curEdit).sprite.size() > 2)
		{
			gameData->mapView->DrawRoom(gameData->rooms.at(curEdit).name); // Draw the room
			textCol = sdl::BLACK;
		}
		Button panelBtn{ .onClick = [&] {
			curEdit = -1;
		}, .text = "Editing room: " + gameData->rooms.at(curEdit).name, .pos = { 0, 0 } };
		panelBtn.Draw();

		if (curEdit == -1) return;

		SDLWrapper::DrawString("sprite: " + gameData->rooms.at(curEdit).sprite, { 0, 20 }, textCol);
		SDLWrapper::DrawString("components: ", { 0, 36 }, textCol);
		int y = 48;
		for (auto& c : gameData->rooms.at(curEdit).components)
		{
			SDLWrapper::DrawString(" - " + c, { 16, y }, textCol);
			y += 16;
		}

		gobl::vec2i rect = { 100, 100 };
		static int _holding = -1;

		if (SDLWrapper::getMouse().bHeld(0) == false) _holding = -1;
		for (int i = 0; i < gameData->rooms.at(curEdit).standOffs.size(); i++)
		{
			auto& s = gameData->rooms.at(curEdit).standOffs.at(i);
			if (SDLWrapper::getMouse().x >= s.x && SDLWrapper::getMouse().x <= s.x + rect.x &&
				SDLWrapper::getMouse().y >= s.y && SDLWrapper::getMouse().y <= s.y + rect.y && SDLWrapper::getMouse().bHeld(0) && _holding == -1) _holding = i;

			SDLWrapper::DrawRect(s.x, s.y, rect.x, rect.y, sdl::RED);
			gameData->suspectSprite.Draw(0, 0, s, rect);
			SDLWrapper::DrawString("standOff: " + std::to_string(s.x) + ", " + std::to_string(s.y), s, sdl::CYAN);
		}

		if (_holding != -1)
		{
			auto& s = gameData->rooms.at(curEdit).standOffs.at(_holding);
			s.x = SDLWrapper::getMouse().x;
			s.y = SDLWrapper::getMouse().y;
		}

		// TODO: Provide tools for editing the existing information

		// TODO: Add new tools for placing the characters
	}
}

bool Game::OnUserUpdate(float deltaTime)
{
	SDLWrapper::SetClear(SDL_Color(96, 128, 255, 255));
	SDLWrapper::getMouse().visible = true;

	if (SDLWrapper::getKeyboard().combo(new int[] { int(SDLK_LCTRL), int(SDLK_LSHIFT), int(SDLK_RCTRL), int(SDLK_RSHIFT) }, 4))
	{
		state = RoomEditing;
	}

	if (SDLWrapper::getKeyboard().bDown(SDLK_TAB))
	{
		// if (state == Investigating) state = Accusing;
		if (state == Accusing) state = Investigating;
	}

	if (state == Investigating)
	{
		holdIndex = -1;
		interviewing = 0;
	}

	int stateEvent = 0;

	switch (state)
	{
	case Game::Introduction:
		DisplayIntroduction(deltaTime);
		break;
	case Game::Interviewing:
		DisplayInterview(deltaTime);
		break;
	case Game::Accusing:
		DisplayAccusing();
		break;
	case Game::Investigating:
		stateEvent = gameData->mapView->Display(deltaTime);
		if (stateEvent == -1) state = Lose;
		else if (stateEvent > 0)
		{
			interviewing = stateEvent - 1;
			state = Interviewing;
		}
		break;
	case Game::Win:
		DisplayKiller(true);
		break;
	case Game::Lose:
		DisplayKiller(false);
		break;

	case Game::RoomEditing:
		DisplayRoomEditor(deltaTime);
		break;
	default:
		break;
	}

	return true;
}

void Game::ApplyData(Loader::GamePack* data)
{
	gameData = data;
}
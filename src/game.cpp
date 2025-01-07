#include "game.hpp"
#include "SDLWrapper.hpp"

#define YAML_DEF
#include "../libs/yaml.hpp"

void Clue::LoadSuspects()
{
	std::vector<std::string> suspectNames{};
	std::vector<std::vector<std::string>> suspectMotives{};
	std::cout << "Loading suspects..." << std::endl;

	YAML::Node root;
	YAML::Parse(root, "config/suspects.yaml");

	std::string sprName = "";
	int sprWidth = 0, sprHeight = 0, sprCols = 0, sprRows = 0;
	killer = 0;
	// Iterate second sequence item.
	for (auto it = root.Begin(); it != root.End(); it++)
	{
		// std::cout << (*it).first << ": " << std::to_string((*it).second.As<int>()) << std::endl;
		if ((*it).first == "width") sprWidth = (*it).second.As<int>();
		else if ((*it).first == "height") sprHeight = (*it).second.As<int>();
		else if ((*it).first == "cols") sprCols = (*it).second.As<int>();
		else if ((*it).first == "spriteName") sprName = (*it).second.As<std::string>();
		else if ((*it).first == "rows") sprRows = (*it).second.As<int>();
		else if ((*it).first == "characters")
		{
			for (auto o = (*it).second.Begin(); o != (*it).second.End(); o++)
			{
				suspectNames.push_back((*o).second["name"].As<std::string>());
				// FIXME: Read for motives next
				for (auto m = (*o).second["motives"].Begin(); m != (*o).second["motives"].End(); m++)
				{
					if (suspectMotives.size() < killer + 1) suspectMotives.push_back(std::vector<std::string>{});
					suspectMotives.at(killer).push_back((*m).second.As<std::string>());
				}

				killer++;
			}
		}
	}

	suspectSprite.Load(sprName, sprWidth, sprHeight, sprCols, sprRows);
	std::cout << "Done loading suspects. Creating cards." << std::endl;

	SDLWrapper::LoadSprite("sprites/player.png");

	for (int i = 0; i < suspectNames.size(); i++) suspects.push_back(Suspect{ suspectNames.at(i), suspectSprite, i, suspectMotives.at(i) });
}

void Clue::LoadWeapons()
{
	std::vector<std::string> names{};
	std::cout << "Loading weapons..." << std::endl;

	YAML::Node root;
	YAML::Parse(root, "config/weapons.yaml");

	std::string sprName = "";
	int sprWidth = 0, sprHeight = 0, sprCols = 0, sprRows = 0;
	// Iterate second sequence item.
	for (auto it = root.Begin(); it != root.End(); it++)
	{
		// std::cout << (*it).first << ": " << std::to_string((*it).second.As<int>()) << std::endl;
		if ((*it).first == "width") sprWidth = (*it).second.As<int>();
		else if ((*it).first == "height") sprHeight = (*it).second.As<int>();
		else if ((*it).first == "cols") sprCols = (*it).second.As<int>();
		else if ((*it).first == "rows") sprRows = (*it).second.As<int>();
		else if ((*it).first == "spriteName") sprName = (*it).second.As<std::string>();
		else if ((*it).first == "names")
		{
			for (auto o = (*it).second.Begin(); o != (*it).second.End(); o++)
				names.push_back((*o).second.As<std::string>());
		}
	}

	weaponSprite.Load(sprName, sprWidth, sprHeight, sprCols, sprRows);
	std::cout << "Done loading weapons. Creating cards." << std::endl;

	for (int i = 0; i < names.size(); i++) weapons.push_back({ names.at(i), weaponSprite, i });
}

bool Clue::DisplaySuspectCards()
{
	gobl::vec2<int> suspectPos = gobl::vec2<int>{ 10, SDLWrapper::getScreenHeight() - (suspectSprite.height + 5) };
	const int SUSPECT_SPACING = (int)std::floor((float)suspectSprite.width * 0.5f);

	for (int i = 0; i < suspects.size(); i++)
	{
		auto suspect = suspects.at(i);
		bool mouseOver = SDLWrapper::getMouse().x < suspectPos.x + suspectSprite.width && SDLWrapper::getMouse().x > suspectPos.x && SDLWrapper::getMouse().y < suspectPos.y + suspectSprite.height && SDLWrapper::getMouse().y > suspectPos.y;

		if (holdIndex == i && holding == SUSPECT)
		{
			sdl::CardRect(SDLWrapper::getMousePos(), gobl::vec2<int>{ suspectSprite.width, suspectSprite.height }, sdl::DARK_GREY);
			suspect.Draw(SDLWrapper::getMousePos(), suspectSprite.width >> 1);

			if (SDLWrapper::getMouse().y > suspectPos.y - 20) suspectPos.x += (SUSPECT_SPACING * 2) + 10;
		}
		else
		{
			if (mouseOver)
			{
				sdl::CardRect(suspectPos - gobl::vec2<int>{ 0, 4 }, { suspectSprite.width, suspectSprite.height }, sdl::DARK_GREY);
				SDLWrapper::DrawString(suspect.name, suspectPos - gobl::vec2<int>{ 0, 16 }, sdl::BLACK);
				suspect.Draw(suspectPos - gobl::vec2<int>{ 0, 4 }, suspectSprite.width >> 1);

				if (SDLWrapper::getMouse().bHeld(0) && holdIndex == -1)
				{
					holdIndex = i;
					holding = SUSPECT;
				}
				suspectPos.x += SUSPECT_SPACING + 10;
			}
			else
			{
				sdl::CardRect(suspectPos, { suspectSprite.width, suspectSprite.height }, sdl::DARK_GREY);
				suspect.Draw(suspectPos, suspectSprite.width >> 1);
			}

			suspectPos.x += SUSPECT_SPACING;
		}
	}

	if (SDLWrapper::getMouse().bRelease(0)) holdIndex = -1;

	return holdIndex != -1 && holding == SUSPECT;
}

bool Clue::DisplayWeaponCards()
{
	gobl::vec2<int> pos = gobl::vec2<int>{ 30, SDLWrapper::getScreenHeight() - (weaponSprite.height + 5) - suspectSprite.height };
	const int SPACING = (int)std::floor((float)weaponSprite.width * 0.5f);

	for (int i = 0; i < weapons.size(); i++)
	{
		auto item = weapons.at(i);
		bool mouseOver = SDLWrapper::getMouse().x < pos.x + weaponSprite.width && SDLWrapper::getMouse().x > pos.x && SDLWrapper::getMouse().y < pos.y + weaponSprite.height && SDLWrapper::getMouse().y > pos.y;

		if (holdIndex == i && holding == WEAPON)
		{
			sdl::CardRect(SDLWrapper::getMousePos(), { weaponSprite.width, weaponSprite.height }, sdl::DARK_GREY);
			item.Draw(SDLWrapper::getMousePos(), weaponSprite.width >> 1);

			if (SDLWrapper::getMouse().y > pos.y - 20) pos.x += (SPACING * 2) + 10;
		}
		else
		{
			if (mouseOver)
			{
				sdl::CardRect(pos - gobl::vec2<int>{ 0, 4 }, { weaponSprite.width, weaponSprite.height }, sdl::DARK_GREY);
				SDLWrapper::DrawString(item.name, pos - gobl::vec2<int>{ 0, 16 }, sdl::BLACK);
				item.Draw(pos - gobl::vec2<int>{ 0, 4 }, weaponSprite.width >> 1);

				if (SDLWrapper::getMouse().bHeld(0) && holdIndex == -1)
				{
					holdIndex = i;
					holding = WEAPON;
				}
				pos.x += SPACING + 10;
			}
			else
			{
				sdl::CardRect(pos, { weaponSprite.width, weaponSprite.height }, sdl::DARK_GREY);
				item.Draw(pos, weaponSprite.width >> 1);
			}

			pos.x += SPACING;
		}
	}

	return holdIndex != -1 && holding == WEAPON;
}

void Clue::DisplayAccusing()
{
	static bool foundWhat = false;
	static int accusing = -1;

	for (auto& q : questions)
	{
		q.Draw();
		if (q.MouseOver() && SDLWrapper::getMouse().bRelease(0))
		{
			if (q.text == "Who?" && holding == SUSPECT)
			{
				q.answer = suspects.at(holdIndex).name;
				accusing = holdIndex;
			}
			if (q.text == "What?" && holding == WEAPON)
			{
				q.answer = weapons.at(holdIndex).name;
				if (q.answer == weapons.at(weapon).name) foundWhat = true;
				else foundWhat = false;
			}
		}

		if (q.text == "Why?" && accusing != -1) q.answer = (suspects.at(accusing).foundMotive) ? suspects.at(accusing).GetMotive() : "???";
		if (q.text == "Where?") q.answer = mapView->GetMurderRoom();
	}

	DisplayWeaponCards();
	DisplaySuspectCards();

	if (SDLWrapper::getMouse().bRelease(0))
	{
		holdIndex = -1;
		holding = NONE;
	}

	static Button btn = { .onClick = [&]() {
		if ((killer == accusing) && foundWhat) state = Win;
		else
		{
			accusing = -1;
			for (auto& q : questions) q.answer = "";
		}
	}, .text = "Accuse", .pos = questions.back().pos + gobl::vec2<int>{ 0, 50} };
	btn.Draw();
}

void Clue::DisplayKiller(bool foundKiller)
{
	gobl::vec2<int> killerPos = { 100, 100 };
	gobl::vec2<int> weaponPos = { 100, 120 + suspectSprite.height };

	int score = 0;

	if (foundKiller)
	{
		suspects.at(killer).Draw(killerPos);
		SDLWrapper::DrawString("The killer was " + suspects.at(killer).name, killerPos + gobl::vec2<int>{ 0, suspectSprite.height }, sdl::BLACK);

		weapons.at(weapon).Draw(weaponPos);
		SDLWrapper::DrawString("with the " + weapons.at(weapon).name, weaponPos + gobl::vec2<int>{ 0, weaponSprite.height }, sdl::BLACK);
		score += 50;
	}
	else SDLWrapper::DrawString("The killer wasn't caught...", killerPos + gobl::vec2<int>{ 0, suspectSprite.height }, sdl::BLACK);

	if (mapView->GetMurderRoom() != "???")
	{
		SDLWrapper::DrawString("they struck in the " + mapView->GetMurderRoom(), weaponPos + gobl::vec2<int>{ 0, weaponSprite.height + 24 }, sdl::BLACK);
		score += 25;
	}
	else SDLWrapper::DrawString("We may never know where...", weaponPos + gobl::vec2<int>{ 0, weaponSprite.height + 24 }, sdl::BLACK);

	if (suspects.at(killer).foundMotive)
	{
		SDLWrapper::DrawString("their motive was " + suspects.at(killer).GetMotive(), weaponPos + gobl::vec2<int>{ 0, weaponSprite.height + 36 }, sdl::BLACK);
		score += 25;
	}
	else SDLWrapper::DrawString("We may never know why...", weaponPos + gobl::vec2<int>{ 0, weaponSprite.height + 36 }, sdl::BLACK);

	std::string grade = "F";
	if (score > 80) grade = "A";
	else if (score > 70) grade = "C";
	else if (score > 0) grade = "D";

	SDLWrapper::DrawString("CASE GRADE: " + grade, gobl::vec2<int>{ SDLWrapper::getScreenWidth() - 220, SDLWrapper::getScreenHeight() - 36 }, sdl::BLACK);
}

void Clue::DisplayInterview(float deltaTime)
{
	gobl::vec2<int> speachBubblePos = gobl::vec2<int>{ 10, SDLWrapper::getScreenHeight() >> 1 };
	gobl::vec2<int> suspectPos = gobl::vec2<int>{ 100, speachBubblePos.y - suspectSprite.height };

	suspects.at(interviewing).Draw(suspectPos);
	SDLWrapper::DrawString(suspects.at(interviewing).name, suspectPos + gobl::vec2<int>{ 0, suspectSprite.height }, sdl::BLACK);
	speachBubblePos.y += 25;
	SDLWrapper::DrawString("How can I help you detective?", speachBubblePos, sdl::WHITE);
	speachBubblePos.y += 25;

	static std::vector<std::string> answers{
		"Why would you have killed Mr. Boddy?", "Nevermind..."
	};
	static QuestionObject responseBox = { .text = "Response ", .pos = gobl::vec2<int>{speachBubblePos.x + 100, speachBubblePos.y + 25} };
	responseBox.Draw();

	if (responseBox.answer.size() < 2)
	{
		gobl::vec2<int> pos = { 5, SDLWrapper::getScreenHeight() - 64 };
		const int SPACING = 15 * 4;

		for (int i = 0; i < answers.size(); i++)
		{
			auto item = answers.at(i);
			bool mouseOver = SDLWrapper::getMouse().x < pos.x + SPACING + 12 && SDLWrapper::getMouse().x > pos.x && SDLWrapper::getMouse().y < pos.y + 64 && SDLWrapper::getMouse().y > pos.y;

			if (holdIndex == i)
			{
				sdl::CardRect(SDLWrapper::getMousePos(), { SPACING + 12, 64 }, sdl::DARK_GREY);
				SDLWrapper::DrawString(item, SDLWrapper::getMousePos() + gobl::vec2<int>{ 0, 16 }, sdl::BLACK);

				if (SDLWrapper::getMouse().y > pos.y - 20) pos.x += (SPACING * 2) + 10;
			}
			else
			{
				if (mouseOver) pos.y -= 10;

				sdl::CardRect(pos, { SPACING + 12, 64 }, sdl::DARK_GREY);
				SDLWrapper::DrawString(item, pos + gobl::vec2<int>{ 0, 16 }, sdl::BLACK);

				if (mouseOver)
				{
					if (SDLWrapper::getMouse().bHeld(0) && holdIndex == -1)
					{
						holdIndex = i;
					}

					pos.y += 10;
					pos.x += SPACING + 10;
				}

				pos.x += SPACING;
			}
		}

		if (SDLWrapper::getMouse().bRelease(0))
		{
			if (responseBox.MouseOver()) responseBox.answer = answers.at(holdIndex);
			holdIndex = -1;
		}
	}
	else
	{
		if (responseBox.answer == answers.back())
		{
			state = Investigating;
			responseBox.answer = "";
			return;
		}

		SDLWrapper::DrawString("I suppose it would be " + suspects.at(interviewing).GetMotive(), speachBubblePos, sdl::BLACK); // FIXME: Do an actual minigame with the interview
		suspects.at(interviewing).foundMotive = true;

		static Button jaccuse = { .onClick = [&]() {
			state = Accusing;
			responseBox.answer = "";
		}, .text = "Accuse", .pos = { SDLWrapper::getScreenWidth() - 100, SDLWrapper::getScreenHeight() - 50} };
		jaccuse.Draw();

		static Button btn = { .onClick = [&]() {
			state = Investigating;
			responseBox.answer = "";
		}, .text = "Done", .pos = { SDLWrapper::getScreenWidth() - 100, SDLWrapper::getScreenHeight() - 25} };
		btn.Draw();
	}
}

void Clue::DisplayIntroduction(float deltaTime)
{
	SDLWrapper::DrawString("There's been a murder...", gobl::vec2<int>{ 10, SDLWrapper::getScreenHeight() >> 1 }, sdl::WHITE);

	static std::vector<std::string> answers{
		"Okay.", "I'm on my way.", "I see...", "Not again!"
	};
	static QuestionObject continueFactor = { .text = "Response ", .pos = {100, (SDLWrapper::getScreenHeight() >> 1) + 50} };
	continueFactor.Draw();

	if (continueFactor.answer.size() < 2)
	{
		gobl::vec2<int> pos = { 5, SDLWrapper::getScreenHeight() - 64 };
		const int SPACING = 15 * 4;

		for (int i = 0; i < answers.size(); i++)
		{
			auto item = answers.at(i);
			bool mouseOver = SDLWrapper::getMouse().x < pos.x + SPACING + 12 && SDLWrapper::getMouse().x > pos.x && SDLWrapper::getMouse().y < pos.y + 64 && SDLWrapper::getMouse().y > pos.y;

			if (holdIndex == i)
			{
				sdl::CardRect(SDLWrapper::getMousePos(), { SPACING + 12, 64 }, sdl::DARK_GREY);
				SDLWrapper::DrawString(item, SDLWrapper::getMousePos() + gobl::vec2<int>{ 0, 16 }, sdl::BLACK);

				if (SDLWrapper::getMouse().y > pos.y - 20) pos.x += (SPACING * 2) + 10;
			}
			else
			{
				if (mouseOver) pos.y -= 10;

				sdl::CardRect(pos, { SPACING + 12, 64 }, sdl::DARK_GREY);
				SDLWrapper::DrawString(item, pos + gobl::vec2<int>{ 0, 16 }, sdl::BLACK);

				if (mouseOver)
				{
					if (SDLWrapper::getMouse().bHeld(0) && holdIndex == -1)
					{
						holdIndex = i;
					}

					pos.y += 10;
					pos.x += SPACING + 10;
				}

				pos.x += SPACING;
			}
		}

		if (SDLWrapper::getMouse().bRelease(0) && holdIndex != -1)
		{
			if (continueFactor.MouseOver()) continueFactor.answer = answers.at(holdIndex);
			holdIndex = -1;
		}
	}
	else
	{
		const float MAX_TIME = 5.0f;
		static float time = 0.0f;
		time += deltaTime;
		SDLWrapper::DrawString("Arriving to crime scene in " + std::to_string(int(ceil(MAX_TIME - time))) + "...", gobl::vec2<int>{ 10, SDLWrapper::getScreenHeight() - 12 }, sdl::WHITE);
		if (time >= MAX_TIME) state = Investigating;
	}
}

bool Clue::OnUserUpdate(float deltaTime)
{
	// SDLWrapper::DrawString(std::to_string(deltaTime), { 0, 8 }, sdl::WHITE);

	if (SDLWrapper::getKeyboard().bDown(SDLK_TAB))
	{
		if (state == Investigating) state = Accusing;
		else if (state == Accusing) state = Investigating;
	}

	if (state == Investigating)
	{
		holdIndex = -1;
		interviewing = 0;
	}

	int stateEvent = 0;

	switch (state)
	{
	case Clue::Introduction:
		DisplayIntroduction(deltaTime);
		break;
	case Clue::Interviewing:
		DisplayInterview(deltaTime);
		break;
	case Clue::Accusing:
		DisplayAccusing();
		break;
	case Clue::Investigating:
		stateEvent = mapView->Display(deltaTime);
		if (stateEvent == -1) state = Lose;
		else if (stateEvent > 0)
		{
			interviewing = stateEvent - 1;
			state = Interviewing;
		}
		break;
	case Clue::Win:
		DisplayKiller(true);
		break;
	case Clue::Lose:
		DisplayKiller(false);
		break;
	default:
		break;
	}

	return true;
}

Clue::Clue()
{
	LoadSuspects();
	LoadWeapons();
	// FIXME: Create a card for the location aswell

	std::cout << "Shuffling..." << std::endl;
	srand(static_cast<unsigned int>(time(NULL)));
	killer = rand() % suspects.size();
	weapon = rand() % weapons.size();

	suspects.at(killer).isKiller = true;

	std::cout << "Picked killer and weapon. Begin game!" << std::endl;

	mapView = new MapView(suspects);
	mapView->weapon = weapons.at(weapon).name;
}
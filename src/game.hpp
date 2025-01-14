#pragma once
#include <vector>
#include <iostream>
#include <cstdint>

#include "card.hpp"
#include "questionObject.hpp"
#include "suspect.hpp"
#include "mapView.hpp"
#include "button.hpp"

#include "loader.hpp"

class Game
{
public:
	enum HoldingType { NONE, SUSPECT, WEAPON };
	void ApplyData(Loader::GamePack* gameData);

	bool OnUserUpdate(float deltaTime);
private:
	std::vector<QuestionObject> questions{
		{.text = "Who?", .pos = {150, 100}},
		{.text = "What?", .pos = {150, 140}},
		{.text = "Where?", .pos = {150, 160}},
		{.text = "Why?", .pos = {150, 180}}
	};

	int interviewing = 0;

	Loader::GamePack* gameData = nullptr;

	enum GameState : uint8_t
	{
		Introduction,
		Interviewing,
		Accusing,
		Investigating,
		Win,
		Lose
	};

	GameState state = Introduction;

	int holdIndex = -1;
	HoldingType holding = NONE;

	void DisplayAccusing();
	void DisplayKiller(bool foundKiller);
	void DisplayInterview(float deltaTime);
	void DisplayIntroduction(float deltaTime);
};
#pragma once
#include "suspect.hpp"
#include <iostream>
#include <vector>

struct Room
{
	char index;
	std::string name;
	std::string sprite;
	std::vector<std::string> components{};
};

class MapView
{
	const int MAP_SCALE = 24;
	char playerRoom;
	bool playerKilled = false;
	int interviewing = -1;
	char weaponRoom = '.';
	char murderRoom = '.';
	bool foundMurderRoom = false;

	std::vector<Suspect>& suspects;
	std::vector<char> suspectPos{};

	std::vector<Room> rooms{};
	Room* getRoom(const char&);

	void DrawCharacters(float deltaTime);

public:
	std::string weapon = "";
	std::string GetMurderRoom();

	int Display(float deltaTime);

	void DrawRoom(char index);
	void DrawRoom(std::string name);

	MapView(std::vector<Suspect>& suspects, std::vector<Room>& roomNames);
};
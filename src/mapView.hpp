#pragma once
#include "suspect.hpp"
#include "SDLWrapper.hpp"
#include <iostream>
#include <vector>

struct Standoff
{
	gobl::vec2i pos;
	int scale;
	short order = 0;
};

struct Prop
{
	std::string name;
	SpriteData sprite;
	gobl::vec2i pos;
	int scale;
	short order = 0;
};

struct Room
{
	char index;
	std::string name;
	std::string sprite;
	std::vector<std::string> components{};
	std::vector<Standoff> standOffs{};
	std::vector<Prop> props{};
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
	float dt;

	std::vector<Suspect>& suspects;
	std::vector<char> suspectPos{};
	std::vector<int> roomOccupation{};

	std::vector<Room> rooms{}; // TODO: Make this a pointer so we don't have 2 separate references to rooms
	Room* getRoom(const char&);
	int getRoomIndex(const char& c);

	void DrawCharacters(float deltaTime);

public:
	std::string weapon = "";
	std::string GetMurderRoom();

	int Display(float deltaTime);

	void DrawRoom(std::string name = "", bool populate = false);

	MapView(std::vector<Suspect>& suspects, std::vector<Room>& roomNames);
};
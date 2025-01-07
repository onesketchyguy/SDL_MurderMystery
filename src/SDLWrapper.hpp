#pragma once
#ifndef SDL_WRAPPER
#define SDL_WRAPPER

#include <vector>
#include <string>
#include <map>
#include <SDL2/SDL.h>

namespace gobl
{
	template <typename T>
	struct vec2
	{
		T x = float(0.0), y = float(0.0);
		vec2() = default;
		vec2(T x, T y) : x(x), y(y) {}
		bool operator==(const vec2& v) { return (x == v.x && y == v.y); }
		bool operator!=(const vec2& v) { return (x != v.x || y != v.y); }
		void operator+=(const vec2& v) { this->x += v.x; this->y += v.y; }
		vec2 operator+(const vec2& v) { return vec2(this->x + v.x, this->y + v.y); }
		void operator-=(const vec2& v) { this->x -= v.x; this->y -= v.y; }
		vec2 operator-(const vec2& v) { return vec2(this->x - v.x, this->y - v.y); }
		void operator*=(const float& v) { this->x *= v; this->y *= v; }
		vec2 operator*(const float& v) { return vec2(this->x * v, this->y * v); }
		void operator/=(const float& v) { this->x /= v; this->y /= v; }
		vec2 operator/(const float& v) { return vec2(this->x / v, this->y / v); }
		const float distance(vec2<float> o) // This is super constly
		{
			auto p = pow(o.x - x, 2);
			auto q = pow(o.y - y, 2);
			return sqrt(p + q);
		}
	};

	typedef vec2<float> vec2f;
	typedef vec2<double> vec2d;
	typedef vec2<int> vec2i;
}

namespace sdl
{
	static const SDL_Color
		GREY(192, 192, 192, 255), DARK_GREY(128, 128, 128, 255), VERY_DARK_GREY(64, 64, 64, 255),
		RED(255, 0, 0, 255), DARK_RED(128, 0, 0, 255), VERY_DARK_RED(64, 0, 0, 255),
		YELLOW(255, 255, 0, 255), DARK_YELLOW(128, 128, 0, 255), VERY_DARK_YELLOW(64, 64, 0, 255),
		GREEN(0, 255, 0, 255), DARK_GREEN(0, 128, 0, 255), VERY_DARK_GREEN(0, 64, 0, 255),
		CYAN(0, 255, 255, 255), DARK_CYAN(0, 128, 128, 255), VERY_DARK_CYAN(0, 64, 64, 255),
		BLUE(0, 0, 255, 255), DARK_BLUE(0, 0, 128, 255), VERY_DARK_BLUE(0, 0, 64, 255),
		MAGENTA(255, 0, 255, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64, 255),
		WHITE(255, 255, 255, 255), BLACK(0, 0, 0, 255), BLANK(0, 0, 0, 0);
}

struct ButtonInterface
{
private:
	std::map<int, int> btns{ };

public:
	void assert(int i, int j)
	{
		if (btns.count(i) > 0) btns[i] = j;
		else btns.emplace(i, j);
	}

	int getRaw(int i)
	{
		if (btns.count(i) <= 0) btns.emplace(i, 0);
		return btns.at(i);
	}

	std::map<int, int>& getAllRaw()
	{
		return btns;
	}

	bool bDown(int i)
	{
		if (btns.count(i) > 0) return btns.at(i) == 1;
		else return 0;
	}

	bool bHeld(int i)
	{
		if (btns.count(i) > 0) return btns.at(i) == 2;
		else return 0;
	}

	bool bRelease(int i)
	{
		if (btns.count(i) > 0) return btns.at(i) == 3;
		else return 0;
	}

	bool bAny(int i)
	{
		if (btns.count(i) > 0) return btns.at(i) > 0;
		else return 0;
	}
};

struct Mouse : public ButtonInterface
{
	int x;
	int y;
};

class SDLWrapper
{
private:
	std::map<std::string, void*> types{};
	std::map<std::string, void*> textures{};

	ButtonInterface keyboard{};
	Mouse mouse{};
	int screenWidth, screenHeight;
	static SDLWrapper* instance;
public:
	SDLWrapper(const char* appName, int width, int height);
	~SDLWrapper();

	static int InitSDL(const char* appName);
	static bool Update();

	static ButtonInterface& getKeyboard() { return instance->keyboard; }
	static Mouse& getMouse() { return instance->mouse; }
	static gobl::vec2i getMousePos() { return gobl::vec2i{ instance->mouse.x, instance->mouse.y }; }

	static const int getScreenHeight() { return instance->screenHeight; }
	static const int getScreenWidth() { return instance->screenWidth; }

	static int LoadSprite(const std::string&);
	static void DrawSprite(const std::string&, gobl::vec2f pos, SDL_Color tint = SDL_Color{ 255U, 255U, 255U, 255 });
	static void DrawSprite(const std::string&, gobl::vec2f pos, gobl::vec2i scale, SDL_Color tint = SDL_Color{ 255U, 255U, 255U, 255 });
	static void DrawSprite(const std::string&, gobl::vec2f pos, gobl::vec2i scale, gobl::vec2i srcPos, gobl::vec2i srcScale, SDL_Color tint = SDL_Color{ 255U, 255U, 255U, 255 });
	static void DrawRect(int x, int y, int w, int h, SDL_Color color = SDL_Color{ 255U, 255U, 255U, 255 });
	static void OutlineRect(int x, int y, int w, int h, SDL_Color color = SDL_Color{ 255U, 255U, 255U, 255 });
	static void DrawCircle(int x, int y, float rad, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255);
	static void OutlineCircle(int x, int y, float rad, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255);
	static void DrawString(const std::string& str, gobl::vec2i pos, SDL_Color color = SDL_Color{ 255U, 255U, 255U, 255 });
	static void DrawLine(gobl::vec2f a, gobl::vec2f b, SDL_Color color = SDL_Color{ 255U, 255U, 255U, 255 });

	static float deltaTime();

	template <typename T>
	static T* getType(const std::string&);
	static void SetType(const std::string&, void*);
};
#endif // SDL_WRAPPER
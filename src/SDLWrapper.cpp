#include "SDLWrapper.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <vector>
SDLWrapper* SDLWrapper::instance = nullptr;

const char* ONE_FONT_PATH = "fonts/Deutsch.ttf"; // FIXME: Move this over to a list of fonts

// MARK: Sprite def
class Renderable
{
public:
	Renderable(int x, int y, int w, int h, SDL_Color col = { 255, 255, 255, 255 }, bool solid = true) :
		dest{ x, y, w, h }, color(col), solid(solid) {}

	typedef enum { RECT, CIRCLE, LINE, SPRITE, TEXT } RenderableType;

	SDL_Rect dest{};
	SDL_Color color{ 255, 255, 255, 255 };
	bool solid = true;

	virtual int GetTypeID() { return RECT; }
};

class Circle : public Renderable
{
private:
	float radius;
public:
	Circle(int x, int y, float radius, SDL_Color col = { 255, 255, 255, 255 }, bool solid = true) :
		Renderable(x, y, 0, 0, col, solid), radius(radius) {}

	const float& getRadius() { return radius; }
	int GetTypeID() override { return CIRCLE; }
};

class Line : public Renderable
{
public:
	Line(float x1, float y1, float x2, float y2, SDL_Color col = { 255, 255, 255, 255 }) :
		Renderable(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2), col) {}

	int GetTypeID() override { return LINE; }
};

class Sprite : public Renderable
{
private:
	std::string pathID;

public:
	const std::string& getID() { return pathID; }
	SDL_Rect src{};

	int GetTypeID() override { return SPRITE; }

	Sprite(const std::string& path, int x, int y, int w, int h, int srcx, int srcy, int srcw, int srch, SDL_Color col = { 255, 255, 255, 255 }, bool solid = true) :
		src{ srcx, srcy, srcw, srch }, pathID(path), Renderable(x, y, w, h, col, solid) {}
};

class Text : public Renderable
{
private:
	std::string text;
public:
	int GetTypeID() override { return TEXT; }

	const std::string& getString() { return text; }

	Text(std::string t, int x, int y, SDL_Color col) : text(t), Renderable(x, y, 0, 0, col, false) {}
};

// MARK: This pass
std::vector<Renderable*> renderables{};

SDLWrapper::SDLWrapper(const char* appName, int width, int height) : screenWidth(width), screenHeight(height)
{
	instance = this;

	InitSDL(appName);
	Update();
}

SDLWrapper::~SDLWrapper()
{
	//Deallocate surface
	for (auto& s : textures) SDL_DestroyTexture((SDL_Texture*)s.second);

	//Destroy window
	SDL_DestroyWindow(getType<SDL_Window>("window"));

	TTF_CloseFont(getType<TTF_Font>(ONE_FONT_PATH));
	TTF_Quit();

	//Quit SDL subsystems
	SDL_Quit();
}

int SDLWrapper::InitSDL(const char* appName)
{
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1; // 1: Indicates a failure with SDL
	}
	else
	{
		int windowHints = SDL_WINDOW_SHOWN, rendererFlags = SDL_RENDERER_ACCELERATED;

		window = SDL_CreateWindow(appName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, instance->screenWidth, instance->screenHeight, windowHints);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return 2; // 2: Indicates a failure with the window
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		renderer = SDL_CreateRenderer(window, -1, rendererFlags);

		if (!renderer)
		{
			printf("Failed to create renderer: %s\n", SDL_GetError());

			SDL_DestroyWindow(window);

			return 3; // 3: Indicates a failure with the renderer
		}
	}

	// Push all our new objects onto the stack
	SetType("renderer", renderer);
	SetType("window", window);

	// Ready graphics stuff now
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

	if (TTF_Init() < 0)
	{
		printf("Couldn't initialize SDL TTF: %s\n", SDL_GetError());
		return 4; // 4: Indicates a failture with the fonts module
	}

	int FONT_SIZE = 16;
	TTF_Font* curFont = TTF_OpenFont(ONE_FONT_PATH, FONT_SIZE);
	SetType(ONE_FONT_PATH, curFont);

	return 0;
}

bool SDLWrapper::Update()
{
	bool appRunning = true;

	for (int i = 1; i < 5; i++)
	{
		if (instance->mouse.getRaw(i - 1) == 1) instance->mouse.assert(i - 1, 2); // Held the button
		if (instance->mouse.getRaw(i - 1) == 3) instance->mouse.assert(i - 1, 0); // Release the button
	}

	for (auto& k : instance->keyboard.getAllRaw())
	{
		if (k.second == 1) k.second = 2; // Held the button
		if (k.second == 3) k.second = 0; // Release the button
	}

	// for ()

	//Handle events on queue
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
	{
		//User requests quit
		if (e.type == SDL_QUIT)
		{
			std::printf("SDL Quit\n");
			appRunning = false;
			// exit(0);
		}
		else if (e.type == SDL_MOUSEMOTION)
		{
			SDL_GetMouseState(&instance->mouse.x, &instance->mouse.y);
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			if (instance->mouse.getRaw(e.button.button - 1) != 2) instance->mouse.assert(e.button.button - 1, 1);
		}
		else if (e.type == SDL_MOUSEBUTTONUP)
		{
			instance->mouse.assert(e.button.button - 1, 3);
		}
		else if (e.type == SDL_KEYDOWN)
		{
			if (instance->mouse.getRaw(e.key.keysym.sym) != 2) instance->keyboard.assert(e.key.keysym.sym, 1);
		}
		else if (e.type == SDL_KEYUP)
		{
			instance->keyboard.assert(e.key.keysym.sym, 3);
		}
	}

	SDL_Renderer* renderer = getType<SDL_Renderer>("renderer");

	// MARK: Render the screen
	if (renderer != nullptr)
	{
		SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255); // Clear the screen
		SDL_RenderClear(renderer);
	}
	else
	{
		std::cout << "No renderer!" << std::endl;
		return false; // Fail exit
	}

	// Render all the renderables
	for (auto& o : renderables)
	{
		SDL_SetRenderDrawColor(renderer, o->color.r, o->color.g, o->color.b, o->color.a);

		if (o->GetTypeID() == Renderable::RECT)
		{
			if (o->solid == true) SDL_RenderFillRect(renderer, &o->dest);
			else SDL_RenderDrawRect(renderer, &o->dest);
		}
		else if (o->GetTypeID() == Renderable::CIRCLE)
		{
			// FIXME: Implement this, as it is it is not funcitonal
			Circle* circle = dynamic_cast<Circle*>(o);
			const int32_t diameter = static_cast<int32_t>(circle->getRadius() * 2);
			float x = static_cast<float>(circle->dest.x), y = static_cast<float>(circle->dest.y);

			int num = 0;
			SDL_FPoint points[360];
			for (int i = 0; i < 360; i++)
			{
				points[num++] = SDL_FPoint{ x + float(circle->getRadius() * cos(i)), y + float(circle->getRadius() * sin(i)) };
			}

			SDL_RenderDrawLinesF(renderer, points, num);
		}
		else if (o->GetTypeID() == Renderable::LINE)
		{
			SDL_FPoint points[2] = { { (float)o->dest.x, (float)o->dest.y }, { (float)o->dest.w, (float)o->dest.h } };
			SDL_RenderDrawLinesF(renderer, points, 2);
		}
		else if (o->GetTypeID() == Renderable::SPRITE)
		{
			Sprite* sprite = dynamic_cast<Sprite*>(o);
			if (sprite != nullptr && instance->textures[sprite->getID()] != nullptr)
			{
				SDL_Texture* texture = static_cast<SDL_Texture*>(instance->textures[sprite->getID()]);
				SDL_RenderCopy(renderer, texture, (sprite->src.w == 0 ? NULL : &sprite->src), &sprite->dest);
				// We don't need to destroy this texture because it's just a reference to a library of textures
			}
		}
		else if (o->GetTypeID() == Renderable::TEXT)
		{
			Text* text = dynamic_cast<Text*>(o);
			SDL_Surface* surface = TTF_RenderUTF8_Blended(getType<TTF_Font>(ONE_FONT_PATH), text->getString().c_str(), text->color);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface); // Cleanup the "surface" object

			SDL_QueryTexture(texture, NULL, NULL, &text->dest.w, &text->dest.h);
			text->dest.y -= text->dest.h >> 1; // FIXME: We don't need to do this, make the user do it
			SDL_RenderCopy(renderer, texture, NULL, &text->dest);
			SDL_DestroyTexture(texture); // We DO need to destroy this texture because it's been created this frame
		}

		delete o;
	}

	renderables.clear();

	// Push everything onto the screen
	SDL_RenderPresent(getType<SDL_Renderer>("renderer"));
	SDL_UpdateWindowSurface(getType<SDL_Window>("window"));

	return appRunning;
}

// MARK: Draw routines
int SDLWrapper::LoadSprite(const std::string& path)
{
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", path.c_str());
	SDL_Texture* texture = IMG_LoadTexture(getType<SDL_Renderer>("renderer"), path.c_str());

	if (instance->textures.count(path) <= 0) instance->textures.emplace(path, texture);
	else
	{
		try
		{
			if (instance->textures[path] != NULL) SDL_DestroyTexture((SDL_Texture*)instance->textures[path]);
		}
		catch (const std::exception& e)
		{
			std::cout << "Failed to delete sprite: " << e.what() << std::endl;
		}
		instance->textures[path] = texture;
	}

	return 0;
}

void SDLWrapper::DrawSprite(const std::string& n, gobl::vec2f pos, SDL_Color col) { DrawSprite(n, pos, gobl::vec2i{ 0, 0 }, col); } // Falls to the next item
void SDLWrapper::DrawSprite(const std::string& n, gobl::vec2f pos, gobl::vec2i scale, SDL_Color col) { DrawSprite(n, pos, scale, gobl::vec2i{ 0, 0 }, gobl::vec2i{ 0, 0 }, col); } // Falls again
void SDLWrapper::DrawSprite(const std::string& n, gobl::vec2f pos, gobl::vec2i scale, gobl::vec2i srcPos, gobl::vec2i srcScale, SDL_Color col)
{
	if (instance->textures.count(n) <= 0)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Cannot draw %s because it has not been loaded!", n.c_str());
		return;
	}

	if (scale.x == 0 && scale.y == 0) SDL_QueryTexture(static_cast<SDL_Texture*>(instance->textures[n]), NULL, NULL, &scale.x, &scale.y); // Load the default size of the texture

	renderables.push_back(new Sprite(n, static_cast<int>(pos.x), static_cast<int>(pos.y), scale.x, scale.y, srcPos.x, srcPos.y, srcScale.x, srcScale.y, col));
}

void SDLWrapper::DrawRect(int x, int y, int w, int h, SDL_Color col)
{
	renderables.push_back(new Renderable(x, y, w, h, col));
}

void SDLWrapper::OutlineRect(int x, int y, int w, int h, SDL_Color col)
{
	renderables.push_back(new Renderable(x, y, w, h, col, false));
}

void SDLWrapper::DrawCircle(int x, int y, float rad, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	renderables.push_back(new Circle(x, y, rad, { r, g, b, a }));
}

void SDLWrapper::OutlineCircle(int x, int y, float rad, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	renderables.push_back(new Circle(x, y, rad, { r, g, b, a }, false));
}

void SDLWrapper::DrawString(const std::string& str, gobl::vec2i pos, SDL_Color col)
{
	renderables.push_back(new Text(str, pos.x, pos.y, col));
}

void SDLWrapper::DrawLine(gobl::vec2f a, gobl::vec2f b, SDL_Color col)
{
	renderables.push_back(new Line(a.x, a.y, b.x, b.y, col));
}

float SDLWrapper::deltaTime()
{
	static int start = SDL_GetTicks();
	int cur = SDL_GetTicks();

	float dt = static_cast<float>(cur - start) / 1000.0f;
	start = cur;

	return dt;
}

// MARK: Generic storage device
template <typename T>
T* SDLWrapper::getType(const std::string& n)
{
	if (instance->types.count(n) > 0) return static_cast<T*>(instance->types[n]);
	else return nullptr;
}
void SDLWrapper::SetType(const std::string& n, void* o)
{
	if (instance->types.count(n) <= 0) instance->types.emplace(n, o);
	else instance->types[n] = o;
}
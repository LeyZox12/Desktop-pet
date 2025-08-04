#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <Windows.h>
#include "../../class/UIutils.h"
#include "include/Pet.hpp"

using namespace sf;
using namespace std;

typedef Vector2f vec2;
typedef Vector2i vec2i;

vec2 winSize = vec2(0, 0);
Texture spriteSheet;
sf::Clock deltaClock;
bool holding = false;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam); 

void start()
{
	if(!spriteSheet.loadFromFile("res/spriteSheet.png")) cout << "failed to load sprite sheet\n";
}

Pet pet("res/spriteSheet.png", 32, vec2(200, 200), vec2(200, 100));
void updatePetWindows();
int main()
{
	start();
	while(pet.window.isOpen())
	{
		float dt = deltaClock.restart().asSeconds();
		while(optional<Event> e = pet.window.pollEvent())
		{
			if(pet.windowOpen) pet.ui.updateElements(e, pet.window);
			if(e->is<Event::KeyPressed>() && e->getIf<Event::KeyPressed>() -> code == Keyboard::Key::Escape) pet.window.close();
			if(e->is<Event::KeyPressed>() && e->getIf<Event::KeyPressed>() -> code == Keyboard::Key::Space) pet.jump();
			else if(e->is<Event::MouseButtonPressed>() && e->getIf<Event::MouseButtonPressed>() -> button == Mouse::Button::Left) holding = true;
			else if(e->is<Event::MouseButtonPressed>() && e->getIf<Event::MouseButtonPressed>() -> button == Mouse::Button::Right) pet.onRightClick();
		}
		if(!Mouse::isButtonPressed(Mouse::Button::Left) && holding)
		{
			holding = false;
			pet.onStopDrag();
		}
		vec2 mousePos = vec2(Mouse::getPosition().x, Mouse::getPosition().y);
		pet.update(mousePos, dt);
		if(holding) pet.drag(mousePos, dt);
		pet.draw();
		updatePetWindows();
	}
	pet.onExitProgram();
	return 0;
}

void updatePetWindows()
{	
	static clock_t end = clock() + 1000;
	if(clock() > end)
	{
		end = clock() + 1000;
		pet.windows.clear();
		EnumWindows(EnumWindowsProc, 0);	
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	if (GetTopWindow(hwnd) == 0 || !IsWindowVisible(hwnd)) return TRUE;

    RECT rect;
    GetWindowRect(hwnd, &rect);
	if(rect.right < 0) return TRUE;
	char title[256];
	GetWindowTextA(hwnd, title, 256);
	pet.windows.push_back(rect);	
    return TRUE;
}

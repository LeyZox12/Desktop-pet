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

CircleShape c(256.f);
std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
RenderWindow window(VideoMode({500, 500}), "Vertex Shaders Testing");
vec2 points[100];
HWND hwnd = static_cast<HWND>(window.getNativeHandle());
vec2 winSize = vec2(0, 0);
Texture spriteSheet;
sf::Clock deltaClock;
bool holding = false;

void start()
{
	window.setVerticalSyncEnabled(true);
	c.setFillColor(Color::Green);
	winSize = vec2(modes[0].size.x, modes[0].size.y);
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	style |= WS_EX_LAYERED;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE,style);
	style = GetWindowLongPtr(hwnd, GWL_STYLE);
	style &= ~WS_BORDER;
	style &= ~WS_CAPTION;
	style &= ~WS_THICKFRAME;
	SetWindowLongPtr(hwnd, GWL_STYLE, style);
	SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);    
	if(!spriteSheet.loadFromFile("res/spriteSheet.png")) cout << "failed to load sprite sheet\n";
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER |SWP_NOSIZE);
	//View view(Rect<float>({0, 0}, {winSize.x + 43, winSize.y}));
	//window.setView(view);
}

Pet pet("res/spriteSheet.png", 32, vec2(200, 200), vec2(200, 100));
int main()
{
	start();
	float dt = deltaClock.restart().asSeconds();
	while(window.isOpen())
	{
		while(optional<Event> e = window.pollEvent())
		{
			if(pet.windowOpen) pet.ui.updateElements(e, window);
			if(e->is<Event::Closed>())window.close();
			if(e->is<Event::KeyPressed>() && e->getIf<Event::KeyPressed>() -> code == Keyboard::Key::Escape) window.close();
			else if(e->is<Event::MouseButtonPressed>() && e->getIf<Event::MouseButtonPressed>() -> button == Mouse::Button::Left) holding = true;
			else if(e->is<Event::MouseButtonPressed>() && e->getIf<Event::MouseButtonPressed>() -> button == Mouse::Button::Right) pet.onRightClick();
		}
		if(!Mouse::isButtonPressed(Mouse::Button::Left)) holding = false;
		pet.update(dt, vec2(modes[0].size.x, modes[0].size.y -150));
		if(holding) pet.drag(vec2(Mouse::getPosition().x, Mouse::getPosition().y), window, dt);
		window.setPosition(Vector2i(pet.pos.x, pet.pos.y));
		window.clear(Color(255, 0, 255));
		pet.draw(window);
		if(pet.windowOpen)
			pet.ui.displayElements(window);
		window.display();

	}
	return 0;
}

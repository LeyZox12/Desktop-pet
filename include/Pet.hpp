#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>


using namespace std;
using namespace sf;

typedef Vector2f vec2;


struct Pet
{
	Pet(string spriteSheetPath, int squareSize, vec2 size, vec2 pos)
	{
		if(!spriteSheet.loadFromFile(spriteSheetPath)) cout << "couldn't load spriteSheet\n";
		this->pos = pos;
		this->squareSize = squareSize;
		sprite.setPosition(vec2(250, 250));
		sprite.setOrigin({size.x * 0.5f, size.y * 0.5f});
		sprite.setSize(size);
		sprite.setTexture(&spriteSheet);
		sprite.setTextureRect(Rect<int>({state * squareSize, 0}, {squareSize, squareSize}));
		this->oldPos = pos;
		vel = vec2(0, 0);
		Font f;
		if(!f.openFromFile("res/font.ttf")) cout << "couldn't load font\n"; 
		ui.font = f;
		ui.addButton(vec2(10, 0), vec2(90, 60), [this](){hotbarOffset+=5;}, "+");
		ui.addButton(vec2(100, 0), vec2(90, 60), [this](){hotbarOffset-=5;}, "-");
		ui.addButton(vec2(10, 70), vec2(50, 50), [this](){windowOpen = false;}, "x");
	}

	void drag(vec2 mousePos, RenderWindow& window, float dt)
	{
		if(windowOpen)return;
		vel.y = 0;
		vec2 size = sprite.getSize();
		sprite.setOrigin({size.x * 0.5f, 0});
		angVel -= (pos.x - mousePos.x + 250) * 0.3f;
		angVel *= 0.95;
		sprite.rotate(degrees(angVel));
		float fixedAngle = sprite.getRotation().asDegrees();
		fixedAngle = fixedAngle > 180 ? -180 + (fixedAngle - 180) : fixedAngle;
		angVel -= dt * fixedAngle * 0.5f;;
		state = 1;
		float angle = sprite.getRotation().asDegrees();
		if(angle < 270 && angVel < 0 && angle > 180) 
		{
			sprite.setRotation(degrees(270));
			angVel = 0;
		}		
		else if(sprite.getRotation().asDegrees() > 90 && angVel > 0 && angle < 180) 
		{
			sprite.setRotation(degrees(90));
			angVel = 0;	
		}
		pos =mousePos - vec2(250, 250);
	}
	
	void onRightClick()
	{
		windowOpen = true;
	}

	void update(float dt, vec2 winSize)
	{
		state = 0;
		vec2 size = sprite.getSize();
		sprite.setOrigin({size.x * 0.5f, size.y * 0.5f});
		vec2 updated = pos * 2.f - oldPos + vel * dt * dt;
		oldPos = pos;
		pos = updated;
		if(pos.y - oldPos.y > 1) state = 2;
		vel.x *= 0.9;
		if(pos.x < 0) pos.x = 0;
		else if(pos.x > winSize.x - size.x) pos.x = winSize.x - size.x;
		if(pos.y + size.y > winSize.y - hotbarOffset)
		{
			pos.y = winSize.y - size.y - hotbarOffset;
			vel.y = 0;
			vel.x = (oldPos.x - pos.x) * 200.f;
		}
		vel.y += 9.8;	
	}
	
	void draw(RenderWindow& window)
 	{
		sprite.setTextureRect(Rect<int>({state * squareSize, 0}, {squareSize, squareSize}));
		window.draw(sprite);
	}
	
	RectangleShape sprite;
	Texture spriteSheet;
	int state = 0;
	bool windowOpen = false;
	int hotbarOffset = 70;
	int squareSize;
	float angVel = 0;
	vec2 oldPos;
	UIutils ui;
	vec2 pos;
	vec2 vel;
};

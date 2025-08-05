#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <thread>
#include <future>

using namespace std;
using namespace sf;

typedef Vector2f vec2;

///TODO make man clib window randomly 
struct Pet
{
	Pet(string spriteSheetPath, int squareSize, vec2 size, vec2 pos)
	:window(VideoMode({2.5f * size.x, 2.5f * size.y}), "")
	{
		//------------------WINDOW INIT----------------------------
		window.setVerticalSyncEnabled(true);
		hwnd = static_cast<HWND>(window.getNativeHandle());
		vector<VideoMode> modes = VideoMode::getFullscreenModes();
		screenSize = vec2(modes[0].size.x, modes[0].size.y);
		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		style |= WS_EX_LAYERED;
		style |= WS_EX_TOPMOST;
		SetWindowLongPtr(hwnd, GWL_EXSTYLE,style);
		style = GetWindowLongPtr(hwnd, GWL_STYLE);
		style &= ~WS_BORDER;
		style &= ~WS_CAPTION;
		style &= ~WS_THICKFRAME;
		SetWindowLongPtr(hwnd, GWL_STYLE, style);
		SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);    
		SetWindowPos(hwnd, HWND_TOPMOST, pos.x, pos.y, 500, 500, 0); 
		//------------------SPRITE INIT----------------------------
		ifstream file("init.dat");
		if(file.good())
		{
			file >> hotbarOffset;
			file >> squareSize;
		}
		else
		{
			hotbarOffset = 70;
			squareSize = 32;
		}
		if(!spriteSheet.loadFromFile(spriteSheetPath)) cout << "couldn't load spriteSheet\n";
		this->pos = pos;
		this->squareSize = squareSize;
		sprite.setPosition(vec2(window.getSize().x * 0.5f, window.getSize().y * 0.5f));
		sprite.setOrigin({size.x * 0.5f, size.y * 0.5f});
		sprite.setSize(size);
		sprite.setTexture(&spriteSheet);
		sprite.setTextureRect(Rect<int>({state * squareSize, 0}, {squareSize, squareSize}));
		this->oldPos = pos;
		vel = vec2(0, 0);
		//------------------CONTEX MENU INIT----------------------
		Font f;
		if(!f.openFromFile("res/font.otf")) cout << "couldn't load font\n"; 
		ui.font = f;
		ui.addButton(vec2(10, 0), vec2(90, 60), [this](){hotbarOffset+=5;}, "+");
		ui.addButton(vec2(100, 0), vec2(90, 60), [this](){hotbarOffset-=5;}, "-");
		ui.addButton(vec2(100, 70), vec2(90, 60), [this]()
		{
			string str = "true";
			if(mode == Mode::FOLLOWCUR)
			{
				str = "false";
				mode = Mode::IDLE;
			}	
			else
				mode = Mode::FOLLOWCUR;
				
			ui.buttons[2].buttonNameStr = "follow:" + str;
		}, "follow: false");
		ui.addButton(vec2(10, 70), vec2(90, 60), [this](){windowClimbIndex = rand()%windows.size();}, "climb");
		ui.addButton(vec2(10, 130), vec2(90, 60), [this](){jump();}, "jump");
		ui.addButton(vec2(10, 190), vec2(50, 50), [this](){pool.push_back(async([this](){closeWindow();}));}, "x");
	}
	
	void drag(vec2 mousePos, float dt)
	{
		if(windowOpen)return;
		sprite.setScale({1, 1});
		mode = Mode::DRAG;
		angVel *= 0.95;
		float diff;
		diff = pos.x - mousePos.x; 
		angVel -= (diff + window.getSize().x * 0.5f) * 0.01f;
		vel.y = 0;
		vec2 size = sprite.getSize();
		sprite.setOrigin({size.x * 0.5f, 0});
		sprite.rotate(degrees(angVel));
		float fixedAngle = sprite.getRotation().asDegrees();
		fixedAngle = fixedAngle > 180 ? -180 + (fixedAngle - 180) : fixedAngle;
		angVel -= dt * fixedAngle * 0.7f;;
		state = 1;
		animating = true;
		animationEndTime = clock() + 10;
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
		oldPos = pos;
		pos =mousePos - vec2(250, 250);
	}
		
	void onExitProgram()
	{
		ofstream file("init.dat");
		file << hotbarOffset << endl;
		file << squareSize;
		file.close();
	}

	void onRightClick()
	{
		windowOpen = true;
	}

	void onStopDrag()
	{
		if(windowOpen) return;
		mode = Mode::IDLE;
		sprite.setRotation(degrees(0));
	}	

	void doSomething()
	{
		this_thread::sleep_for(chrono::milliseconds(1000+rand()%5000));
		cout << "bro did something\n"; 
		switch(mode)
		{
			case(Mode::IDLE):
			break;	
		}
	}

	void jump()
	{
		switch(mode)
		{
			case(Mode::IDLE):
				vel.y = -JUMP_FORCE;
				pos += vel;
				oldPos = pos - vel;
			break;
			case(Mode::CLIMB):
				sprite.setScale({1, 1});
				if(windowClimbIndex == Side::LEFT)
				{
					vel.y = 0;
					vel.x = JUMP_FORCE * 0.5f;
					mode = Mode::IDLE;
				}
				else if(windowClimbIndex == Side::RIGHT)
				{
					vel.y = 0;
					vel.x = -JUMP_FORCE * 0.5f;
					mode = Mode::IDLE;
				}
				oldPos = pos-vel;
				pos += vel;
			break;
		}
	}

	void updatePhysics(float dt)
	{
		if(windowOpen)return;
		float halfWin = window.getSize().x * 0.5f;
		float halfSprite = sprite.getSize().x * 0.5f;
		int yDiff = pos.y - oldPos.y;
		int xDiff = pos.x - oldPos.x;
		vec2 size = sprite.getSize();
		sprite.setOrigin({size.x * 0.5f, size.y * 0.5f});
		vec2 updated = pos * 2.f - oldPos + vel * dt * dt;
		oldPos = pos;
		pos = updated;
		if(yDiff > 3 && xDiff < yDiff) 
		{
			sprite.setScale({xDiff > 0 ? 1 : -1, 1});
			sprite.setRotation(degrees(0));
			state = 2;
			animating = true;
			animationEndTime = clock() + 10;
		}
		else if(yDiff < -3 && xDiff > yDiff)
		{
			sprite.setRotation(degrees(0));
			sprite.setScale({1, 1});
			state = 4;
			animating = true;
			animationEndTime = clock() + 10;
		}
		else if(xDiff > 3)
		{
			state = 2;
			sprite.setRotation(degrees(90));
			sprite.setScale({-1, -1});
			animating = true;
			animationEndTime = clock() + 10;
		}
		else if(xDiff < -3)
		{
			state = 2;
			sprite.setRotation(degrees(90));
			sprite.setScale({-1, 1});
			animating = true;
			animationEndTime = clock() + 10;

		}
		vel.x *= 0.9;
		if(pos.x < halfSprite - halfWin)
		{
			pos.x = halfSprite - halfWin;
			windowClimbIndex = Side::LEFT;
			mode = Mode::CLIMB;
			pool.push_back(async([this](){onWallHit();}));
		}
		else if(pos.x > screenSize.x - halfWin - halfSprite)
		{
			pos.x = screenSize.x - halfWin - halfSprite;
			windowClimbIndex = Side::RIGHT;
			mode = Mode::CLIMB;
			pool.push_back(async([this](){onWallHit();}));
		}
		if(pos.y + size.y > screenSize.y - hotbarOffset)
		{
			if(yDiff >= 3)
			{
				state = 3;
				animationEndTime = clock() + 1000;
				animating = true;
			}
			pos.y = screenSize.y - size.y - hotbarOffset;
			vel.y = 0;
			vel.x = (oldPos.x - pos.x) * FRICTION;
		}
		vel.y += 9.8;
	}

	void update(vec2 mousePos, float dt)
	{
		float halfWin = window.getSize().x * 0.5f;
		float halfSprite = sprite.getSize().x * 0.5f;
		int yDiff = pos.y - oldPos.y;
		int xDiff = pos.x - oldPos.x;
		switch(mode)
		{		
			case(Mode::DRAG):
				state = 1;
			break;
			case(Mode::FOLLOWCUR):
				vel.x += ((mousePos.x - halfWin - pos.x) < 0 ? -1 : 1) * moveSpeed;
				updatePhysics(dt);
				break;
			case(Mode::IDLE):
			{
				if(animating && clock() > animationEndTime)
				{
					sprite.setScale({1, 1});
					sprite.setRotation(degrees(0));
					animating = false;
					state = 0;
				}
				updatePhysics(dt);
			}	
			break;
			case(Mode::CLIMB):
			{
				if(windowClimbIndex < 0)
				{
					int side = windowClimbIndex == Side::LEFT ? 1 : -1;
					if(state != 5)
					{
						if(abs(xDiff) > 3 && !animating)
						{
							state = 3;
							animationEndTime = clock() + 1000;
							animating = true;
						}
						else if(!animating)
						{
							state = 5;
							sprite.setScale({side, 1});
							sprite.setRotation(degrees(0));
						}
					}
					if(animating && clock() > animationEndTime)
					{
						sprite.setScale({side, 1});
						sprite.setRotation(degrees(0));
						animating = false;
						state = 5;
					}
				}
				else
				{
					RECT& rect = windows[windowClimbIndex];
					int side = abs(pos.x - rect.left) < abs(pos.x - rect.right) ? -1 : 1;
					
				}

			}
			break;
		}
		SetWindowPos(hwnd, NULL, pos.x, pos.y, 500, 500, 0); 
		
		//-----------------clearing thread pool--------------------
		vector<int> toRemove;
		for(int i = pool.size()-1; i > 0; i--)
			if(pool[i].wait_for(0ms) == future_status::ready)
				toRemove.push_back(i);
		for(auto& index : toRemove)
			pool.erase(pool.begin() + index);
	}
	
	void draw()
 	{	
		window.clear(Color(255, 0, 255));
		sprite.setTextureRect(Rect<int>({state * squareSize, 0}, {squareSize, squareSize}));
		window.draw(sprite);
		if(windowOpen)
			ui.displayElements(window);
		window.display();
	}
	
	void closeWindow()
	{
		this_thread::sleep_for(1500ms);//timer to prevent instant grab after clicking button
		windowOpen = false;
	}

	void onWallHit()
	{
		animating = false;
		state = 3;
		sprite.setScale({-1, windowClimbIndex == Side::RIGHT ? -1 : 1});
		sprite.setRotation(degrees(90));
		this_thread::sleep_for(500ms);
		Mode::CLIMB;
		this_thread::sleep_for(chrono::milliseconds(1500+rand()%1000));
		jump();
	}
	
	enum Mode
	{
		IDLE,
		FOLLOWCUR,
		CLIMB,
		DRAG
	};

	enum Side
 	{
		LEFT = -1,
		RIGHT = -2
	};

	RectangleShape sprite;
	Texture spriteSheet;
	int state = 0;
	int mode = 0;
	int windowClimbIndex;
	
	clock_t animationEndTime;
	bool animating = false;
	float moveSpeed = 2000;
	bool windowOpen = false;
	vector<RECT> windows;
	int hotbarOffset;
	int squareSize;
	const float JUMP_FORCE = 5;
	const float FRICTION = 600;
	float angVel = 0;
	vec2 oldPos;
	UIutils ui;
	vec2 pos;
	vec2 screenSize;
	RenderWindow window;
	vector<future<void>> pool;
	HWND hwnd;
	vec2 vel;
};

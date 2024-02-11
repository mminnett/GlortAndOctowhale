// Button art reference: http://opengameart.org/content/ui-pack (public domain)
//

/*

	Author: Matthew Minnett
	Date: 2022-11-08
	Purpose: Work with movement and collision in a 2D environment

*/

#include "MyProject.h"
#include <Windowsx.h> // for GET__LPARAM macros
#include <SpriteBatch.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <DirectXColors.h>
#include <sstream>
#include "Collision2D.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// helper function
//
// returns a random float between 0 & 1
float RandFloat() { return float(rand())/float(RAND_MAX); } 

//----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
	MyProject application(hInstance);    // Create the class variable

	if( application.InitWindowsApp(L"BREAKOUT!", nShowCmd) == false )    // Initialize the window, if all is well show and update it so it displays
	{
		return 0;                   // Error creating the window, terminate application
	}

	if( application.InitializeDirect3D() )
	{
		application.SetDepthStencil(true);      // Tell DirectX class to create and maintain a depth stencil buffer
		application.InitializeTextures();
		application.MessageLoop();				// Window has been successfully created, start the application message loop
	}

	return 0;
}

//----------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------
MyProject::MyProject(HINSTANCE hInstance)
	: DirectXClass(hInstance)
{
	mousePos = Vector2(clientWidth * 0.5f, clientHeight * 0.5f);
	currentState = gameStates::START;
	buttonDown = false;
	keyDown = false;
	powerSpeed = false;
	powerSlow = false;
	score = 0;
	blocksRemaining = NUM_BLOCKS;
	scoreMultiplier = 1;
	lives = 3;
	powerSpot1 = 0;
	powerSpot2 = 0;
	powerSpot3 = 0;
	powerSpot3 = 0;
	powerSpot4 = 0;
	powerSpot5 = 0;
	powerSpot6 = 0;
	powerSpot7 = 0;
	ballSpeed = 100;
	speedyPower = 50;
	paddleSpeed = 150;
	difficultyScaler = 2;
	livesColor = Color(1, 1, 1);
	spriteBatch = NULL;

	ClearColor = Color(DirectX::Colors::DarkGray.v);
}

//----------------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------------
MyProject::~MyProject()
{
	delete spriteBatch;
}

//----------------------------------------------------------------------------------------------
// Initialize any fonts we need to use here
//----------------------------------------------------------------------------------------------
void MyProject::InitializeTextures()
{
	// initialize the sprite batch
	spriteBatch = new DirectX::SpriteBatch( DeviceContext );

	commonStates = new CommonStates(D3DDevice);

	srand((int)time(0));

	// Extremely ugly while loop to assign a powerup to 7 random blocks. They cannot be the same.
	while (powerSpot1 == powerSpot2 || powerSpot1 == powerSpot3 || powerSpot1 == powerSpot4 || powerSpot1 == powerSpot5 || powerSpot1 == powerSpot6 || powerSpot1 == powerSpot7 ||
			powerSpot2 == powerSpot3 || powerSpot2 == powerSpot4 || powerSpot2 == powerSpot5 || powerSpot2 == powerSpot6 || powerSpot2 == powerSpot7 ||
			powerSpot3 == powerSpot4 || powerSpot3 == powerSpot5 || powerSpot3 == powerSpot6 || powerSpot3 == powerSpot7 ||
			powerSpot4 == powerSpot5 || powerSpot4 == powerSpot6 || powerSpot4 == powerSpot7 ||
			powerSpot5 == powerSpot6 || powerSpot5 == powerSpot7 ||
			powerSpot6 == powerSpot7)
	{
		powerSpot1 = rand() % NUM_BLOCKS;
		powerSpot2 = rand() % NUM_BLOCKS;
		powerSpot3 = rand() % NUM_BLOCKS;
		powerSpot4 = rand() % NUM_BLOCKS;
		powerSpot5 = rand() % NUM_BLOCKS;
		powerSpot6 = rand() % NUM_BLOCKS;
		powerSpot7 = rand() % NUM_BLOCKS;
	}

	// Loading textures
	// Menus/Buttons
	startTex.Load(D3DDevice, L"..\\Textures\\start.png");
	rulesTex.Load(D3DDevice, L"..\\Textures\\rules.png");
	loseTex.Load(D3DDevice, L"..\\Textures\\lose.png");
	winTex.Load(D3DDevice, L"..\\Textures\\win.png");
	buttonPlayTex.Load(D3DDevice, L"..\\Textures\\buttonPlay.png");
	buttonRulesTex.Load(D3DDevice, L"..\\Textures\\buttonRules.png");
	buttonExitTex.Load(D3DDevice, L"..\\Textures\\buttonExit.png");
	buttonMenuTex.Load(D3DDevice, L"..\\Textures\\buttonMenu.png");

	// Gameplay Textures
	backgroundTex.Load(D3DDevice, L"..\\Textures\\background.png");
	ballTex.Load(D3DDevice, L"..\\Textures\\glortSpritesheet.png");
	paddleTex.Load(D3DDevice, L"..\\Textures\\octowhaleSpritesheet.png");
	// Various block textures (default, damaged, powers)
	blockTex.Load(D3DDevice, L"..\\Textures\\morloxSpritesheet.png");
	blockDamageTex.Load(D3DDevice, L"..\\Textures\\morloxDamagedSpritesheet.png");
	blockSpeedyTex.Load(D3DDevice, L"..\\Textures\\morloxPowerSpritesheet01.png");
	blockSlowTex.Load(D3DDevice, L"..\\Textures\\morloxPowerSpritesheet02.png");
	blockLifeTex.Load(D3DDevice, L"..\\Textures\\morloxPowerSpritesheet03.png");

	// Initializing sprites
	ballSprite.Initialize(&ballTex, Vector2(clientWidth * 0.5, clientHeight * 0.65), 0, 1.3f, Color(1, 1, 1), 0);
	ballSprite.SetVelocity(Vector2(ballSpeed, -ballSpeed), ballSpeed);
	ballSprite.SetTextureAnimation(36, 35, 8);

	paddleSprite.Initialize(&paddleTex, Vector2(clientWidth * 0.5, clientHeight * 0.85), 0, 1.0f, Color(1, 1, 1), 0);
	paddleSprite.SetVelocity(Vector2(0, 0), 0);
	paddleSprite.SetTextureAnimation(379, 63, 8);

	// For loop to initialize button sprites
	for (int i = 0; i < 4; i++)
	{
		switch(i)
		{
			case 0:
				menuButtons[i].Initialize(&buttonPlayTex, Vector2(buttonPlayTex.GetWidth() * 0.5, clientHeight * 0.8), 0, 1.0f, Color(1, 1, 1), 0);
				break;
			case 1:
				menuButtons[i].Initialize(&buttonExitTex, Vector2(clientWidth - buttonExitTex.GetWidth() * 0.5, 50), 0, 1.0f, Color(1, 1, 1), 0);
				break;
			case 2:
				menuButtons[i].Initialize(&buttonRulesTex, Vector2(buttonRulesTex.GetWidth() * 0.5, clientHeight * 0.933), 0, 1.0f, Color(1, 1, 1), 0);
				break;
			case 3:
				menuButtons[i].Initialize(&buttonMenuTex, Vector2(buttonMenuTex.GetWidth() * 0.5, clientHeight * 0.933), 0, 1.0f, Colors::DarkBlue.v, 0);
		}
	}

	// For loop to initialize block sprites
	Vector2 pos = Vector2(180, 50); // starting position
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		blockDamageSprites[i].Initialize(&blockDamageTex, Vector2(-100, 0), 0, 1.0f, Color(1, 1, 1), 0); // another array of damaged block sprites

		// if statements check if the block is powered
		if (i == powerSpot1 || i == powerSpot4 || i == powerSpot6) // speed power blocks
		{
			blockSprites[i].Initialize(&blockSpeedyTex, pos, 0, 1.0f, Color(1, 1, 1), 0);
			blockDamage[i] = 3;
		}
		else if (i == powerSpot2 || i == powerSpot5 || i == powerSpot7) // slow power blocks
		{
			blockSprites[i].Initialize(&blockSlowTex, pos, 0, 1.0f, Color(1, 1, 1), 0);
			blockDamage[i] = 3;
		}
		else if (i == powerSpot3) // life power block
		{
			blockSprites[i].Initialize(&blockLifeTex, pos, 0, 1.0f, Color(1, 1, 1), 0);
			blockDamage[i] = 3;
		}
		else // otherwise blocktexture is set to default
		{
			blockSprites[i].Initialize(&blockTex, pos, 0, 1.0f, Color(1, 1, 1), 0);
			blockDamage[i] = 2;
		}
		blockSprites[i].SetTextureAnimation(85, 74, 8);
		blockDamageSprites[i].SetTextureAnimation(85, 74, 8);

		if (i == 7 || i == 15 || i == 23 || i == 31 || i == 39 || i == 47) // for blocks 8, 16, 24, 32, 40, and 48
		{
			pos.x = 180; // place on starting x-point
			pos.y += blockSprites[i].GetHeight(); // move down on y-axis
		}
		else
		{
			pos.x += blockSprites[i].GetWidth() + 10; // spread them out along the x-axis
		}
	}

	// initialize font
	pixel30.InitializeFont(D3DDevice, DeviceContext, L"..\\Font\\pixel30.spritefont");
}

//----------------------------------------------------------------------------------------------
// Window message handler
//----------------------------------------------------------------------------------------------
LRESULT MyProject::ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam)
{
	Vector2 newVelocity = Vector2(0, 0);

	switch (msg )
	{
	case WM_MOUSEMOVE:
		mousePos.x = (float) GET_X_LPARAM(lParam);
		mousePos.y = (float) GET_Y_LPARAM(lParam);
		return 0;
	case WM_LBUTTONUP:
		buttonDown = false;
		mousePos.x = (float) GET_X_LPARAM(lParam);
		mousePos.y = (float) GET_Y_LPARAM(lParam);
		break;
	case WM_LBUTTONDOWN:
		buttonDown = true;
		mousePos.x = (float) GET_X_LPARAM(lParam);
		mousePos.y = (float) GET_Y_LPARAM(lParam);
		OnMouseDown();
		break;
	case WM_KEYUP:
		keyDown = false;
		paddleSprite.SetVelocity(Vector2(0, 0), 0);
		if (wParam >= '0' && wParam <= '4')
		{
			PresentInterval = wParam - '0';
		}
		break;
	case WM_KEYDOWN:
		keyDown = true;
		if (wParam == VK_LEFT || wParam == 'A')
		{
			newVelocity.x = -paddleSpeed;

			paddleSprite.SetVelocity(newVelocity, 0);
		}
		else if (wParam == VK_RIGHT || wParam == 'D')
		{
			newVelocity.x = paddleSpeed;

			paddleSprite.SetVelocity(newVelocity, 0);
		}
		break;
	}

	// let the base class handle remaining messages
	return DirectXClass::ProcessWindowMessages(msg, wParam, lParam);
}

//----------------------------------------------------------------------------------------------
// Called by the render loop to render a single frame
//----------------------------------------------------------------------------------------------
void MyProject::Render(void)
{
	if (currentState == gameStates::START) // render the menu
	{
		startTex.Draw(DeviceContext, BackBuffer, 0, 0);

		spriteBatch->Begin(SpriteSortMode_BackToFront, commonStates->NonPremultiplied());

		for (int i = 0; i < 3; i++) // for loop to draw each button (play, rules, exit)
		{
			menuButtons[i].Draw(spriteBatch);
		}

		spriteBatch->End();
	}
	else if (currentState == gameStates::RULES) // render the rules screen
	{
		rulesTex.Draw(DeviceContext, BackBuffer, 0, 0);

		menuButtons[1].SetPosition(menuButtons[2].GetPosition()); // set position of exit button to rules button position

		spriteBatch->Begin(SpriteSortMode_BackToFront, commonStates->NonPremultiplied());

		for (int i = 0; i < 2; i++)
		{

			menuButtons[i].Draw(spriteBatch); // draw play and exit buttons

		}

		spriteBatch->End();
	}
	else if (currentState == gameStates::PLAYING) // render game
	{
		backgroundTex.Draw(DeviceContext, BackBuffer, 0, 0);

		spriteBatch->Begin(SpriteSortMode_BackToFront, commonStates->NonPremultiplied());

		// draw sprites
		ballSprite.Draw(spriteBatch);
		paddleSprite.Draw(spriteBatch);

		// for loop to draw each block (+ damaged blocks)
		for (int i = 0; i < NUM_BLOCKS; i++)
		{
			blockSprites[i].Draw(spriteBatch);
			blockDamageSprites[i].Draw(spriteBatch);
		}

		spriteBatch->End();

		// Display score
		std::wostringstream scoreTxt;
		scoreTxt << L"Score: " << score;
		pixel30.PrintMessage(0, clientHeight - 45, scoreTxt.str(), Color(1, 1, 1));

		// Display lives
		std::wostringstream livesTxt;
		livesTxt << L"Lives: " << lives;
		pixel30.PrintMessage(clientWidth - 250, clientHeight - 45, livesTxt.str(), livesColor);

		// render the base class
		DirectXClass::Render();
	}
	else if (currentState == gameStates::OVER) // if game has ended
	{
		if (lives == 0) // lose
		{
			loseTex.Draw(DeviceContext, BackBuffer, 0, 0);
		}
		else if (blocksRemaining == 0) // win
		{
			winTex.Draw(DeviceContext, BackBuffer, 0, 0);
		}

		spriteBatch->Begin(SpriteSortMode_BackToFront, commonStates->NonPremultiplied());

		menuButtons[3].Draw(spriteBatch); // draw menu button

		spriteBatch->End();

		// Display score
		std::wostringstream scoreTxt;
		scoreTxt << L"Final Score: " << score;
		pixel30.PrintMessage(0, clientHeight * 0.75, scoreTxt.str(), Color(1, 1, 1));
	}
}

//----------------------------------------------------------------------------------------------
// Called every frame to update objects.
//	deltaTime: how much time in seconds has elapsed since the last frame
//----------------------------------------------------------------------------------------------
void MyProject::Update(float deltaTime)
{
	// Menu
	if (currentState == gameStates::START)
	{
		// for loop, checks to see if mouse is hovering over buttons
		for (int i = 0; i < 3; i++)
		{
			if (menuButtons[i].ContainsPoint(mousePos))
			{
				Color setColor = Colors::Green; // if yes, highlight button
				menuButtons[i].SetColor(setColor);
			}
			else
			{
				Color setColor = Colors::White; // otherwise don't highlight
				menuButtons[i].SetColor(setColor);
			}
		}
	}

	// Rules screen
	else if (currentState == gameStates::RULES)
	{
		// once again checks if button is being hovered over
		for (int i = 0; i < 3; i++)
		{
			if (menuButtons[i].ContainsPoint(mousePos))
			{
				Color setColor = Colors::Green;
				menuButtons[i].SetColor(setColor);
			}
			else
			{
				Color setColor = Colors::White;
				menuButtons[i].SetColor(setColor);
			}
		}
	}

	// Playing
	else if (currentState == gameStates::PLAYING)
	{
		if (powerTime > 0) // if a power is active
		{
			powerTime -= deltaTime; // powertime counts down
			
			if (powerTime <= 0) // if powertime reaches 0, reset values affected by power
			{
				powerTime = 0;
				paddleSprite.SetScale(1.0f);
				ballSprite.SetScale(1.3f);
				if (powerSpeed == true)
				{
					ballSpeed -= speedyPower;
					paddleSpeed -= speedyPower * 2;
				}
				else if (powerSlow == true)
				{
					ballSpeed += speedyPower;
					paddleSpeed += speedyPower;
				}
				powerSpeed = false;
				powerSlow = false;
			}
		}

		// Ball movement
		MoveBall(deltaTime);

		// Paddle movement
		if (keyDown) // if a, d or left, right are pressed
		{
			MovePaddle(deltaTime);
		}

		// Update Animations
		for (int i = 0; i < NUM_BLOCKS; i++)
		{
			blockSprites[i].UpdateAnimation(deltaTime);
			blockDamageSprites[i].UpdateAnimation(deltaTime);
		}
		ballSprite.UpdateAnimation(deltaTime);
		paddleSprite.UpdateAnimation(deltaTime);

		// Collisions
		CollisionCheck(deltaTime);
	}

	// Game Over
	else if (currentState == gameStates::OVER)
	{
		// Highlights button if hovered over
		if (menuButtons[3].ContainsPoint(mousePos))
		{
			Color setColor = Colors::White;
			menuButtons[3].SetColor(setColor);
		}
		else
		{
			Color setColor = Colors::DarkBlue;
			menuButtons[3].SetColor(setColor);
		}
	}
}

//----------------------------------------------------------------------------------------------
// Called when the mouse is released
//----------------------------------------------------------------------------------------------
void MyProject::OnMouseDown()
{
	// this is called when the left mouse button is clicked
	// mouse position is stored in mousePos variable
	
	// Menu or Rules
	if (currentState == gameStates::START || currentState == gameStates::RULES)
	{
		for (int i = 0; i < 3; i++) // for loop, checks if buttons are clicked
		{
			if (menuButtons[i].ContainsPoint(mousePos))
			{
				switch (i)
				{
				case 0:
					currentState = gameStates::PLAYING; // if play selected
					break;
				case 1:
					exit(0); // if exit selected
					break;
				case 2:
					if (currentState == gameStates::START)
					{
						currentState = gameStates::RULES; // if rules selected
					}
					break;
				}
			}
		}
	}

	// Game Over
	else if (currentState == gameStates::OVER)
	{
		if (menuButtons[3].ContainsPoint(mousePos))
		{
			Reset(); // If menu selected
		}
	}
}

//----------------------------------------------------------------------------------------------
// Called every frame to move ball
//----------------------------------------------------------------------------------------------
void MyProject::MoveBall(float deltaTime)
{
	// Update ball position and rotation
	Vector2 pos = ballSprite.GetPosition();
	Vector2 velocity = ballSprite.GetVelocity(); // if collision, then velocity will update too
	float rotation = ballSprite.GetRotation();
	float rotationVelocity = ballSprite.GetRotationalVelocity();

	// add to position based on current velocity
	pos += velocity * deltaTime;
	ballSprite.SetPosition(pos);

	// add to rotation based on rotational velocity of ball
	rotation += rotationVelocity * deltaTime;
	ballSprite.SetRotation(rotation);

	// Bounce off left
	if (pos.x < ballTex.GetWidth() * 0.5)
	{
		pos.x = ballTex.GetWidth() * 0.5;
		ballSprite.SetPosition(pos);
		velocity.x = -velocity.x;
		rotationVelocity = -rotationVelocity;
	}

	// Bounce off right
	else if (pos.x > clientWidth - ballTex.GetWidth() * 0.5)
	{
		pos.x = clientWidth - ballTex.GetWidth() * 0.5;
		ballSprite.SetPosition(pos);
		velocity.x = -velocity.x;
		rotationVelocity = -rotationVelocity;
	}

	// Bounce off top
	else if (pos.y < ballTex.GetHeight() * 0.5)
	{
		pos.y = ballTex.GetHeight() * 0.5;
		ballSprite.SetPosition(pos);
		velocity.y = -velocity.y;
		rotationVelocity = -rotationVelocity;
	}

	// Bounce off bottom
	else if (pos.y > clientHeight - ballTex.GetHeight() * 0.5)
	{
		lives--; // lose a life for hitting bottom of screen
		scoreMultiplier = 1; // score multiplier reset

		// color of lives text changes based on lives remaining
		if (lives == 2)
		{
			livesColor = Color(1, 0.64, 0); // orange
		}
		else if (lives == 1)
		{
			livesColor = Color(1, 0, 0); // red
		}
		else if (lives == 0)
		{
			currentState = gameStates::OVER; // out of lives, game over!
		}

		pos.y = clientHeight - ballTex.GetHeight() * 0.5;
		ballSprite.SetPosition(pos);
		velocity.y = -velocity.y;
		rotationVelocity = -rotationVelocity;
	}

	// set updated velocity of ball (if no collision, velocity is the same as it was)
	ballSprite.SetVelocity(velocity, rotationVelocity);
}

//----------------------------------------------------------------------------------------------
// Called when the a or d is pressed
//----------------------------------------------------------------------------------------------
void MyProject::MovePaddle(float deltaTime)
{
	Vector2 paddlePos = paddleSprite.GetPosition();
	paddlePos += paddleSprite.GetVelocity() * deltaTime; // update position based on velocity of paddle

	// Preventing paddle from going off screen on left or right side
	if (paddlePos.x < paddleSprite.GetWidth() * 0.5) // left
	{
		paddlePos.x = paddleSprite.GetWidth() * 0.5;
	}

	else if (paddlePos.x > clientWidth - paddleSprite.GetWidth() * 0.5) // right
	{
		paddlePos.x = clientWidth - paddleSprite.GetWidth() * 0.5;
	}

	paddleSprite.SetPosition(paddlePos);
}


//----------------------------------------------------------------------------------------------
// Checks for collisions between ball and blocks/paddle
//----------------------------------------------------------------------------------------------
void MyProject::CollisionCheck(float deltaTime)
{
	Vector2 pos = ballSprite.GetPosition();
	Vector2 velocity = ballSprite.GetVelocity();
	float rotationVelocity = ballSprite.GetRotationalVelocity();

	// Block collisions
	Circle ballCollision(pos, ballSprite.GetWidth() * 0.5f);
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		Box2D blockCollision(blockSprites[i].GetPosition(), blockSprites[i].GetExtents());

		// if collision between a block and the ball
		if (Collision2D::BoxCircleCheck(blockCollision, ballCollision))
		{
			score += 10 * scoreMultiplier; // add to score
			blockDamage[i]--; // Block takes damage

			// if block still has health, and is not powered,
			if (blockDamage[i] > 0 && i != powerSpot1 && i != powerSpot2 && i != powerSpot3 && i != powerSpot4 && i != powerSpot5 && i != powerSpot6 && i != powerSpot7)
			{
				blockDamageSprites[i].SetPosition(blockSprites[i].GetPosition()); // move damage sprite on screen
				blockSprites[i].SetColor(Color(1, 1, 1, 0)); // make default invisible
			}
			else if (blockDamage[i] <= 0) // if block is out of health
			{
				blockDamageSprites[i].SetPosition(Vector2(-100, 0)); // move damage sprite off screen again
				blockSprites[i].SetPosition(Vector2(-100, 0)); // move block off screen
				blocksRemaining--; // blocks remaining decreases
				scoreMultiplier++; // score multiplier increases
			}

			if (blocksRemaining <= 0)
			{
				currentState = gameStates::OVER; // No more blocks, game over!
			}

			ballCollision.center -= velocity * deltaTime;

			pos = Collision2D::ReflectCircleBox(ballCollision, velocity, deltaTime, blockCollision);
			rotationVelocity = -rotationVelocity; // rotation velocity changes

			ballSprite.SetPosition(pos); // update position of ball
			ballSprite.SetVelocity(velocity, rotationVelocity); // update velocity of ball

			Vector2 newSpeed;
			ballSpeed += difficultyScaler; // add difficulty scaler to ball and paddle speed. Game gets faster and faster as player progresses
			paddleSpeed += difficultyScaler;

			// Speedy Power (small and fast)
			if (i == powerSpot1 || i == powerSpot4 || i == powerSpot6)
			{
				if (blockDamage[i] == 2)
				{
					blockSprites[i].SetColor(Color(0.8, 0.8, 0.8)); // first hit
				}
				else if (blockDamage[i] == 1)
				{
					blockSprites[i].SetColor(Color(0.5, 0.5, 0.5)); // second hit
				}
				else if (blockDamage[i] <= 0) // third hit, destroyed
				{
					if (powerSlow) // ensure that other power is disabled
					{
						ballSpeed += speedyPower;
						paddleSpeed += speedyPower;
					}
					else if (powerSpeed) // ensure that this power is disabled (before re-enabling)
					{
						ballSpeed -= speedyPower;
						paddleSpeed -= speedyPower * 2;
					}

					score += 10; // add 10 to score
					ballSpeed += speedyPower; // add speed power to ball speed
					paddleSpeed += speedyPower * 2; // add speed power x 2 to paddle speed

					powerTime = 10.0f; // lasts 10 seconds
					ballSprite.SetScale(1.0f); // ball and paddle shrink
					paddleSprite.SetScale(0.5f);

					powerSpeed = true;
					powerSlow = false;
				}

			}

			// Slow-mo power (big and slow)
			else if (i == powerSpot2 || i == powerSpot5 || i == powerSpot7)
			{
				if (blockDamage[i] == 2)
				{
					blockSprites[i].SetColor(Color(0.8, 0.8, 0.8)); // first hit
				}
				else if (blockDamage[i] == 1)
				{
					blockSprites[i].SetColor(Color(0.5, 0.5, 0.5)); // second hit
				}
				else if (blockDamage[i] <= 0) // third hit, destroyed
				{
					if (powerSpeed) // ensure other power is disabled
					{
						ballSpeed -= speedyPower;
						paddleSpeed -= speedyPower * 2;
					}
					else if (powerSlow) // ensure this power is disabled (before re-enabling)
					{
						ballSpeed += speedyPower;
						paddleSpeed += speedyPower;
					}

					score += 10; // add 10 to score
					ballSpeed -= speedyPower; // ball and paddle speed are slowed
					paddleSpeed -= speedyPower;

					powerTime = 10.0f; // lasts 10 seconds
					paddleSprite.SetScale(1.5f); // paddle and ball grow in size
					ballSprite.SetScale(1.5f);

					powerSpeed = false;
					powerSlow = true;
				}
			}

			else if (i == powerSpot3) // if third power brick type
			{
				if (blockDamage[i] == 2)
				{
					blockSprites[i].SetColor(Color(0.8, 0.8, 0.8)); // first hit
				}
				else if (blockDamage[i] == 1)
				{
					blockSprites[i].SetColor(Color(0.5, 0.5, 0.5)); // second hit
				}
				else if (blockDamage[i] <= 0) // destroyed
				{
					lives++; // extra life. yay!
				}
			}

			// set newspeed of ball, ensuring it goes in the proper direction on x and y
			if (velocity.x > 0)
			{
				newSpeed.x = ballSpeed;
			}
			else if (velocity.x < 0)
			{
				newSpeed.x = -ballSpeed;
			}

			if (velocity.y > 0)
			{
				newSpeed.y = ballSpeed;
			}
			else if (velocity.y < 0)
			{
				newSpeed.y = -ballSpeed;
			}
			// set new velocity (new speed)
			ballSprite.SetVelocity(newSpeed, ballSpeed);
		}
	}

	// Paddle collision
	Box2D paddleCollision(paddleSprite.GetPosition(), paddleSprite.GetExtents());

	// if collision between ball and paddle
	if (Collision2D::BoxCircleCheck(paddleCollision, ballCollision))
	{
		ballCollision.center -= velocity * deltaTime;

		pos = Collision2D::ReflectCircleBox(ballCollision, velocity, deltaTime, paddleCollision);
		rotationVelocity = -rotationVelocity; // rotational velocity changes

		// update position and velocity of ball
		ballSprite.SetPosition(pos);
		ballSprite.SetVelocity(velocity, rotationVelocity);
	}
}

//----------------------------------------------------------------------------------------------
// Resets game data
//----------------------------------------------------------------------------------------------
void MyProject::Reset()
{
	currentState = gameStates::START;
	buttonDown = false;
	keyDown = false;
	powerSpeed = false;
	powerSlow = false;
	score = 0;
	blocksRemaining = NUM_BLOCKS;
	scoreMultiplier = 1;
	lives = 3;
	powerSpot1 = 0;
	powerSpot2 = 0;
	powerSpot3 = 0;
	powerSpot3 = 0;
	powerSpot4 = 0;
	powerSpot5 = 0;
	powerSpot6 = 0;
	powerSpot7 = 0;
	ballSpeed = 100;
	speedyPower = 50;
	paddleSpeed = 150;
	livesColor = Color(1, 1, 1);

	InitializeTextures();
}
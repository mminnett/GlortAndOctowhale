#ifndef _MyProject_h
#define _MyProject_h
#include  <CommonStates.h>
#include "DirectX.h"
#include "Font.h"
#include "TextureType.h"
#include "Sprite.h"
#include "Collision2D.h"

//GAME 1201 Term Assignment 1

// forward declare the sprite batch
namespace DirectX { class SpriteBatch; };

struct Block
{
	Sprite sprite;
	bool   enabled;
};


//----------------------------------------------------------------------------------------------
// Main project class
//	Inherits the directx class to help us initalize directX
//----------------------------------------------------------------------------------------------

class MyProject : public DirectXClass
{
public:
	// constructor
	MyProject(HINSTANCE hInstance);
	~MyProject();

	// Initialize any Textures we need to use
	void InitializeTextures();

	// window message handler
	LRESULT ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam);

	// Called by the render loop to render a single frame
	void Render(void);

	// Called by directX framework to allow you to update any scene objects
	void Update(float deltaTime);

	void MoveBall(float deltaTime);

	void MovePaddle(float deltaTime);

	void CollisionCheck(float deltaTime);

	void Reset();

private:
	static const int NUM_BLOCKS = 48;
	static enum gameStates { START, RULES, PLAYING, OVER };		// Game State enumerated type

	// font variables
	FontType pixel30;

	// Menu textures / Sprites
	TextureType startTex;
	TextureType rulesTex;
	TextureType loseTex;
	TextureType winTex;
	TextureType buttonPlayTex;
	TextureType buttonRulesTex;
	TextureType buttonExitTex;
	TextureType buttonMenuTex;

	Sprite menuButtons[4];

	// Gameplay textures / Sprites
	TextureType backgroundTex;
	TextureType ballTex;
	TextureType paddleTex;
	TextureType blockTex;
	TextureType blockDamageTex;
	TextureType blockSpeedyTex;
	TextureType blockSlowTex;
	TextureType blockLifeTex;

	Sprite ballSprite;
	Sprite paddleSprite;
	Sprite blockSprites[NUM_BLOCKS];
	Sprite blockDamageSprites[NUM_BLOCKS];
	int blockDamage[NUM_BLOCKS];

	// Score / Lives variables
	int score;
	int scoreMultiplier;
	int blocksRemaining;
	int lives;
	Color livesColor;

	// power brick variables
	float powerTime;
	bool powerSlow;
	bool powerSpeed;
	int powerSpot1;
	int powerSpot2;
	int powerSpot3;
	int powerSpot4;
	int powerSpot5;
	int powerSpot6;
	int powerSpot7;

	// speed variables
	int ballSpeed;
	int paddleSpeed;
	int difficultyScaler;
	int speedyPower;

	gameStates currentState;

	// sprite batch 
	DirectX::SpriteBatch* spriteBatch;

	DirectX::CommonStates* commonStates;

	// mouse variables
	Vector2 mousePos;
	bool buttonDown;

	// keyboard variables
	bool keyDown;

	// call when the mouse is released
	void OnMouseDown();

};

#endif
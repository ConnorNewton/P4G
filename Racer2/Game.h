#ifndef GAME_H
#define GAME_H

#include <vector>
#include <future>

#include "Mesh.h"
#include "Model.h"
#include "FX.h"
#include "SpriteFont.h"
#include "Input.h"
#include "Obstacle.h"

//wrap up common behaviors, initialization+shutdown
class Game
{
public:
	//start up and shut down
	Game() {}
	~Game() {
		Release();
	}
	//game logic, called constantly, elapsed time passed in
	void Update(float dTime);
	//render images, called constantly, elapsed time passed in
	void Render(float dTime);
	//called when ALT+ENTER or drag
	void OnResize(int screenWidth, int screenHeight);
	void Initialise();
	void Release();
	//message handling
	LRESULT WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	float scoreX;
	
	//camera
	DirectX::SimpleMath::Vector3 mCamPos   = DirectX::SimpleMath::Vector3(0, 0, -55);
	DirectX::SimpleMath::Vector3 mCamAngle = DirectX::SimpleMath::Vector3(0, 1, 0);

	DirectX::SimpleMath::Vector3 mCarRot = DirectX::SimpleMath::Vector3(0, 0, 0);
	DirectX::SimpleMath::Vector3 mCarPos = DirectX::SimpleMath::Vector3(0, -9, 0);

	//ensure each mesh is loaded and release exactly once
	MeshManager mMeshMgr;
	//game models that reference meshes
	Model mCar;
	Model mCar2;

	//handy rendering state
	FX::MyFX mFX;

private:
	Game& operator=(const Game&) = delete;
	Game(const Game& m) = delete;

	float timeAlive = 0.0;
	float speed = 0.0;
	int lastObs = 0;

	//load meshes
	void Load();
	//display a message
	void LoadDisplay(float dTime);
	//light position
	DirectX::SimpleMath::Vector3 mLightPos = DirectX::SimpleMath::Vector3(0, 0, 0);
	//text
	DirectX::SpriteBatch *mpSpriteBatch = nullptr;
	DirectX::SpriteFont *mpFont = nullptr, *mpFont2 = nullptr;
	
	int rotSlots[6];
	float spawnTimer;

	//loading handler
	struct LoadData
	{
		//second thread
		std::future<void> loader;
		//how many meshes to load
		int totalToLoad = 0;
		//tally
		int loadedSoFar = 0;
		//is the loader active
		bool running = false;
	};
	struct Obstacle
	{
		DirectX::SimpleMath::Vector3 pos = DirectX::SimpleMath::Vector3(100, 0, 0);
		int rotSlot = 0;
		bool active = false;
		Model ObsModel;
	};

	Obstacle Obstacles[5];

	LoadData mLoadData;
	MouseAndKeys mMKInput;
	Gamepad mGamepad;
};

#endif
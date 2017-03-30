#include <sstream>
#include <thread>
#include <iomanip>

#include "WindowUtils.h"
#include "D3D.h"
#include "Game.h"
#include "GeometryBuilder.h"
#include "CommonStates.h"
#include "AudioMgr.h"


using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;





void Game::OnResize(int screenWidth, int screenHeight)
{
	
	OnResize_Default(screenWidth, screenHeight);

	scoreX = screenWidth - 140;
}


void Game::Load()
{
	GetIAudioMgr()->Initialise();
	mLoadData.loadedSoFar++;

	Mesh& ecar = mMeshMgr.CreateMesh("ferrari");
	ecar.CreateFrom("data/Lamborghini.x", gd3dDevice, mFX.mCache);
	mCar.Initialise(ecar);
	mCar.GetScale()= Vector3(3.f, 3.f, 3.f);
	mCar.GetMesh().GetSubMesh(1).material.gfxData.Set(Vector4(1,1,1,1), Vector4(1,1,1,1), Vector4(0.125f, 0.125f, 0.05f, 5));  //body has a touch of speculr shinyness
	mCar.GetMesh().GetSubMesh(0).material.gfxData.Set(Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1), Vector4(0, 0, 0, 1));  //tyres are not shiny!
	mLoadData.loadedSoFar++;

	Mesh& ecar2 = mMeshMgr.CreateMesh("tunnel");
	ecar2.CreateFrom("data/finalTunnel.fbx", gd3dDevice, mFX.mCache);
	mCar2.Initialise(ecar2);
	mCar2.GetPosition() = Vector3(0, 0, 290);
	mCar2.GetRotation() = Vector3(0, 0,0);  // D2R(180)
	mCar2.GetScale() = Vector3(1.f, 1.f, 5.f);
	//mCar2.GetMesh().GetSubMesh(0).material.gfxData.Set(Vector4(1,1,1,1), Vector4(1,1,1,1), Vector4(0.125f, 0.125f, 0.05f, 5));  //body has a touch of speculr shinyness
	//mCar2.GetMesh().GetSubMesh(0).material.gfxData.Set(Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1), Vector4(0, 0, 0, 1));  //tyres are not shiny!
	mLoadData.loadedSoFar++;

	//Mesh& obsMesh = BuildCube(mMeshMgr);
	Mesh& obsMesh = mMeshMgr.CreateMesh("ferrari");
	obsMesh.CreateFrom("data/Lamborghini.x", gd3dDevice, mFX.mCache);
	for (int i = 0; i < 5; ++i)
	{
		Obstacles[i].ObsModel.Initialise(obsMesh);
		Obstacles[i].ObsModel.GetScale() = Vector3(1, 1, 1);

	}
	mLoadData.loadedSoFar++;
}

void Game::LoadDisplay(float dTime)
{
	BeginRender(Colours::White);

	mpSpriteBatch->Begin();

	static int pips = 0;
	static float elapsed = 0;
	elapsed += dTime;
	if (elapsed > 0.25f){
		pips++;
		elapsed = 0;
	}
	if (pips > 10)
		pips = 0;
	wstringstream ss;
	ss << L"Loading meshes(" << (int)(((float)mLoadData.loadedSoFar / (float)mLoadData.totalToLoad)*100.f) << L"%) ";
	for (int i = 0; i < pips; ++i)
		ss << L'.';
	mpFont2->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(100, 200), Colours::Black, 0, Vector2(0, 0), Vector2(1.f, 1.f));

	ss.str(L"");
	ss << L"FPS:" << (int)(1.f / dTime);
	mpFont->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(10, 10), Colours::Black, 0, Vector2(0, 0), Vector2(1.f, 1.f));

	mpSpriteBatch->End();

	EndRender();
}

void Game::Initialise()
{
	//srand(time(NULL));
	mFX.Init(gd3dDevice);

	int rotSlots[] = { 0, 300, 60, 180, 120, 240, 162, 25, 270, 183, 10, 285, 340 };
	/*FX::SetupDirectionalLight(0, true, Vector3(-0.7f, -0.7f, 0.7f), Vector3(0.9f, 0.85f, 0.85f), Vector3(0.1f, 0.1f, 0.1f), Vector3(1, 1, 1));*/
	FX::SetupDirectionalLight(0, true, Vector3(0.2f, 0.2f, 1), Vector3(0.9f, 0.85f, 0.85f), Vector3(0.3f, 0.3f, 0.3f), Vector3(1, 1, 1));


	mpSpriteBatch = new SpriteBatch(gd3dImmediateContext);
	assert(mpSpriteBatch);
	mpFont = new SpriteFont(gd3dDevice, L"data/comicSansMS.spritefont");
	assert(mpFont);

	mpFont2 = new SpriteFont(gd3dDevice, L"data/algerian.spritefont");
	assert(mpFont2);

	mLoadData.totalToLoad = 2;
	mLoadData.loadedSoFar = 0;
	mLoadData.running = true;
	mLoadData.loader = std::async(launch::async, &Game::Load, this);

	mMKInput.Initialise(GetMainWnd());
	mGamepad.Initialise();
}

void Game::Release()
{
	mFX.Release();
	mMeshMgr.Release();
	delete mpFont;
	mpFont = nullptr;
	delete mpSpriteBatch;
	mpSpriteBatch = nullptr;
	delete mpFont2;
	mpFont2 = nullptr;
}

void Game::Update(float dTime)
{
	mGamepad.Update();
	GetIAudioMgr()->Update();

	const float rotInc = 100.f * dTime;

	if (mMKInput.IsPressed(VK_A))
		mCarRot.z -= rotInc * (PI / 180);
	else if (mMKInput.IsPressed(VK_D))
		mCarRot.z += rotInc * (PI / 180);

	if (mLoadData.running)
		return;

	mCamPos.x += mGamepad.GetState(0).leftStickX * dTime;
	mCamPos.z += mGamepad.GetState(0).leftStickY * dTime;
	mCamPos.y += mGamepad.GetState(0).rightStickY * dTime;

	timeAlive += dTime;
	spawnTimer -= dTime;
	if (speed < 50)
		speed += dTime * 10;
	else
		speed += dTime / 2;

	if ((speed > 50) && (spawnTimer <= 0))
	{
		switch (lastObs)
		{
		case 0:
		{
			if (!Obstacles[1].active)
			{
				Obstacles[1].pos = DirectX::SimpleMath::Vector3(0, -7, 100);
				Obstacles[1].ObsModel.GetRotation() = DirectX::SimpleMath::Vector3(0, 0, D2R(rotSlots[(int)(rand() % (13 + 1))]));
				Obstacles[1].active = true;
				lastObs = 1;
			}

		}
		break;
		case 1:
		{
			if (!Obstacles[2].active)
			{
				Obstacles[2].ObsModel.GetRotation() = DirectX::SimpleMath::Vector3(0, 0, 180);
				Obstacles[2].pos = DirectX::SimpleMath::Vector3(0, -7, 100);
				Obstacles[2].ObsModel.GetRotation() = DirectX::SimpleMath::Vector3(0, 0, D2R(rotSlots[(int)(rand() % (13 + 1))]));
				Obstacles[2].active = true;
				lastObs = 2;
			}
		}
		break;
		case 2:
		{
			if (!Obstacles[3].active)
			{
				Obstacles[3].pos = DirectX::SimpleMath::Vector3(0,-7, 100);
				Obstacles[3].ObsModel.GetRotation() = DirectX::SimpleMath::Vector3(0, 0, D2R(rotSlots[(int)(rand() % (13 + 1))]));
				Obstacles[3].active = true;
				lastObs = 3;
			}
		}
		break;
		case 3:
		{
			if (!Obstacles[4].active)
			{
				Obstacles[4].pos = DirectX::SimpleMath::Vector3(0, -7, 100);
				Obstacles[4].ObsModel.GetRotation() = DirectX::SimpleMath::Vector3(0, 0, D2R(rotSlots[(int)(rand() % (13 + 1))]));
				Obstacles[4].active = true;
				lastObs = 4;
			}
		}
		break;
		case 4:
		{
			if (!Obstacles[0].active)
			{
				Obstacles[0].pos = DirectX::SimpleMath::Vector3(0, -7, 100);
				Obstacles[0].ObsModel.GetRotation() = DirectX::SimpleMath::Vector3(0, 0, D2R(rotSlots[(int)(rand() % (13 + 1))]));
				Obstacles[0].active = true;
				lastObs = 0;
			}
		}
		break;
		}
		spawnTimer = maxTimer;
	}


	for (int i = 0; i < 5; i++)
	{
		if (Obstacles[i].active) 
		{
			if (Obstacles[i].pos.z > mCamPos.z)
			{
				Obstacles[i].pos.z -= 0.1;

			}
			else
			{
				Obstacles[i].active = false;
			}

		}
		Obstacles[i].ObsModel.GetPosition() = Obstacles[i].pos;		//TRANSLATE ALL POSITIONS INTO ACTUAL TRANSFORMATIONS
	}
}


void Game::Render(float dTime)
{
	if (mLoadData.running)
	{
		if (!mLoadData.loader._Is_ready())
		{
			LoadDisplay(dTime);
			return;
		}
		mLoadData.loader.get();
		mLoadData.running = false;
		return;
	}

	BeginRender(Colours::Black);

	FX::SetPerFrameConsts(gd3dImmediateContext, mCamPos);

	CreateProjectionMatrix(FX::GetProjectionMatrix(), 0.375f*PI, GetAspectRatio(), 1, 1000.f);
	CreateViewMatrix(FX::GetViewMatrix(), mCamPos, Vector3(0, 0, 0), mCamAngle);

	//CreateProjectionMatrix(FX::GetProjectionMatrix(), 0.25f*PI, GetAspectRatio(), 1, 1000.f);
	//CreateViewMatrix(FX::GetViewMatrix(), Vector3(0, 0, -6), Vector3(0, 0, 0), Vector3(0, 1, 0));
	
	mCamAngle.x = sin(-mCarRot.z);
	mCamAngle.y = cos(-mCarRot.z);
	mCar.GetRotation() = mCarRot;
	mCar.GetPosition() = mCarPos;
	mFX.Render(mCar, gd3dImmediateContext);
	mFX.Render(mCar2, gd3dImmediateContext);
	for (int i = 0; i < 5; i++)
	{
		if (Obstacles[i].active)
			mFX.Render(Obstacles[i].ObsModel, gd3dImmediateContext);
	}
	


	//FX::SetupSpotLight(1, true, (mCarPos + Vector3(0, 8, 0)), Vector3(0, 0, 1), Vector3(0.9f, 0.85f, 0.85f), Vector3(1.f, 0.3f, 0.3f), Vector3(1, 1, 1), 60, 0.1f, D2R(1), D2R(50));
	FX::SetupSpotLight(1, true, (mCarPos + Vector3(0, 8, 0)), Vector3(0, 0, 1), Vector3(0.4f, 0.4f, 0.4f), Vector3(0.1f, 0.1f, 0.1f), Vector3(0, 0, 0), 10, 0.05f, D2R(1), D2R(20));
	CommonStates state(gd3dDevice);
	mpSpriteBatch->Begin(SpriteSortMode_Deferred, state.NonPremultiplied());

	//general messages
		wstringstream ss;
		ss << L"Survived for: " << fixed << setprecision(1) << timeAlive;
		mpFont->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(10, 10), Colours::Green, 0, Vector2(0, 0), Vector2(1.f, 1.f));

		wstringstream ssScore;
		ssScore << L"Score: " << fixed << setprecision(0) << timeAlive * 10;
		mpFont->DrawString(mpSpriteBatch, ssScore.str().c_str(), Vector2(scoreX, 10), Colours::Green, 0, Vector2(0, 0), Vector2(1.f, 1.f));

	mpSpriteBatch->End();


	EndRender();

	mMKInput.PostProcess();
}

LRESULT Game::WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const float camInc = 500.f * GetElapsedSec();
	//do something game specific here
	switch (msg)
	{
		// Respond to a keyboard event.
	case WM_CHAR:
		switch (wParam)
		{
		case 27:
		case 'q':
		case 'Q':
			PostQuitMessage(0);
			return 0;
		case 'u':
			mCamPos.x -= camInc;
			break;
		case 'j':
			mCamPos.x += camInc;
			break;
		case 'i':
			mCamPos.y += camInc;
			break;
		case 'k':
			mCamPos.y -= camInc;
			break;
		case 'o':
			mCamPos.z += camInc;
			break;
		case 'l':
			mCamPos.z -= camInc;
			break;
		case 'r':
			mCamPos = Vector3(0, 0, -55);
			break;
		}
	case WM_INPUT:
			mMKInput.MessageEvent((HRAWINPUT)lParam);
			break;
	}
	//default message handling (resize window, full screen, etc)
	return DefaultMssgHandler(hwnd, msg, wParam, lParam);
}
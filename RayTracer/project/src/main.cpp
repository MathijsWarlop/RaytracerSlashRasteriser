//External includes
#ifdef ENABLE_VLD
#include "vld.h"
#endif
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "Scene.h"
#include "Vector3.h"
#include "Vector4.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - Warlop Mathijs",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	int currentScene = 1;
	
	const auto pSceneTest = new Scene_W3_TestScene2();
	const auto pSceneRefrence = new Scene_W4_RefrenceScene();
	const auto pSceneBunny = new Scene_W4_BunnyScene();
	
	
	pSceneTest->Initialize();
	pSceneRefrence->Initialize();
	pSceneBunny->Initialize();

	//Start loop
	pTimer->Start();

	// Start Benchmark
	// pTimer->StartBenchmark();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ToggleShadow(); 
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pRenderer->CycleLightingMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					if (currentScene == 1)
					{
						currentScene = 2;
					}
					else if (currentScene == 2)
					{
						currentScene = 0;
					}
					else currentScene = 1;
				}
				break;
			}
		}

		//--------- Update ---------
		if (currentScene == 1)
		{
			pSceneRefrence->Update(pTimer);
		}
		else if (currentScene == 2)
		{
			pSceneBunny->Update(pTimer);
		}
		else pSceneTest->Update(pTimer);
		

		//--------- Render ---------
		if (currentScene == 1)
		{
			pRenderer->Render(pSceneRefrence);
			//std::cout << currentScene << std::endl;
		}
		else if (currentScene == 2)
		{
			pRenderer->Render(pSceneBunny);
		}
		else pRenderer->Render(pSceneTest);
		//pRenderer->Render(pScene);

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			//std::cout << currentScene << std::endl;
			
		}
		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}

		//TESTS

		//float dotResult{};
		//dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitY); 
		//std::cout << dotResult << std::endl;

		//Vector3 CrossResult{};
		//CrossResult = Vector3::Cross(Vector3::UnitX, Vector3::UnitZ);
		//std::cout << CrossResult.x << CrossResult.y << CrossResult.z << std::endl;

		
	}
	pTimer->Stop();

	//Shutdown "framework"
	//delete pScene;
	delete pSceneTest;
	delete pSceneBunny;
	delete pSceneRefrence;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}


#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

#include <iostream>


namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}
		
		int mouseX{}, mouseY{};
		int currMouseX{}, currMouseY{};

		const int movementSpeed{ 10 };
		const float rotationSpeed{ 0.3f };
		float aspectRatio{};

		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix projectionMatrix{};
		Matrix viewMatrix{};

		void Initialize(float screenWidth, float screenHeight, float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			aspectRatio = screenWidth / screenHeight;

			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{
			// Create rotation matrix from pitch and yaw
			Matrix TransformMatrix = TransformMatrix.CreateRotation(totalPitch, totalYaw, 0);

			// Update forward vector
			forward = TransformMatrix.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			// Calculate right and up vectors
			right = Vector3::Cross(Vector3::UnitY, forward);
			right.Normalize();
			up = Vector3::Cross(forward, right);
			up.Normalize();

			
			// Construct the inverse view matrix (camera to world)
			viewMatrix = Matrix{
				{right.x, right.y, right.z, 0},
				{up.x, up.y, up.z, 0},
				{forward.x, forward.y, forward.z, 0},
				{origin.x, origin.y, origin.z, 1}
			};

			// Calculate the view matrix (world to camera)
			invViewMatrix = viewMatrix;
			invViewMatrix.Inverse();
			// Debug outputs for validation
			
			//std::cout << origin.x << std::endl;
			// Optional: Remove if rotation is dynamic
			//totalPitch = 0;
			//totalYaw = 0;
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3
			const float far = 1000.0f;
			const float near = 1.0f;
			// Construct the projection matrix
			projectionMatrix = Matrix{
				{ 1 / (aspectRatio * fov),  0,  0,  0 },
				{ 0, 1 / fov,  0,  0 },
				{ 0, 0, far / (far - near), 1 },
				{ 0, 0, -(far * near) / (far - near), 0 }
			};

		}

		void Update(Timer* pTimer)
		{
			

			//Camera Update Logic
			//...
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			
				if (pKeyboardState != nullptr)
				{



				//std::cout << "deez";
				if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
				{
					origin += forward * deltaTime * movementSpeed;
				}
				if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
				{
					origin -= forward * deltaTime * movementSpeed;
				}
				if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
				{
					origin += right * deltaTime * movementSpeed;
				}
				if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
				{
					origin -= right * deltaTime * movementSpeed;
				}
				//if (pKeyboardState[SDL_SCANCODE_UP])
				//{
				//	origin += up * deltaTime * movementSpeed;
				//}
				//if (pKeyboardState[SDL_SCANCODE_DOWN])
				//{
				//	origin -= up * deltaTime * movementSpeed;
				//}




				//Mouse Input


				const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
				if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_RIGHT) && (SDL_GetMouseState(&mouseX, &mouseY)& SDL_BUTTON(SDL_BUTTON_LEFT)))
				{
					origin += Vector3{ 0,1,0 } * deltaTime * (currMouseY - mouseY) * movementSpeed;  
				}
				else if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					totalYaw -= (currMouseX - mouseX) * rotationSpeed * deltaTime * 3;
					totalPitch += (currMouseY - mouseY) * rotationSpeed * deltaTime * 3;
				}
				else if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					origin += forward * deltaTime * (currMouseY - mouseY) * movementSpeed;
					totalYaw -= (currMouseX - mouseX) * rotationSpeed * deltaTime * 3;
					//totalYaw -= (currMouseX - mouseX) * rotationSpeed * deltaTime;
					//origin += right * deltaTime * (currMouseX - mouseX) * movementSpeed;
					//totalPitch += (currMouseY - mouseY) * rotationSpeed * deltaTime;
					//origin -= up * deltaTime * (currMouseY - mouseY) * movementSpeed;
				}

				currMouseX = mouseX, currMouseY = mouseY;
				//std::cout << totalPitch << std::endl;


				//todo: W2
				//throw std::runtime_error("Not Implemented Yet");

			}
			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}

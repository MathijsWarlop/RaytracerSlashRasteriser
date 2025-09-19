#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>
#include "Maths.h"
#include "Timer.h"
#include "Renderer.h"

namespace dae
{
    struct Camera
    {
        Camera() = default;

        Camera(const Vector3& _origin, float _fovAngle) :
            origin{ _origin },
            fovAngle{ _fovAngle }
        {
        }

        int mouseX{}, mouseY{};
        int currMouseX{}, currMouseY{};

        Vector3 origin{};
        float fovAngle{ 90.f };

        float totalPitch{ 0.0f };
        float totalYaw{ 0.0f };

        Vector3 forward{ Vector3::UnitZ };
        Vector3 up{ Vector3::UnitY };
        Vector3 right{ Vector3::UnitX };

        Matrix cameraToWorld{};
        Matrix transformationMatrix{};

        float baseMovementSpeed{ 4.f };
        float boostMultiplier{ 4.f }; // LSHIFT boost
        const float rotationSpeed{ 0.003f };

        Matrix CalculateCameraToWorld()
        {
            // Build rotation from yaw and pitch
            transformationMatrix = Matrix::CreateRotation(totalPitch, totalYaw, 0.f);

            forward = transformationMatrix.TransformVector(Vector3::UnitZ);
            forward.Normalize();

            right = Vector3::Cross(Vector3::UnitY, forward);
            right.Normalize();

            up = Vector3::Cross(forward, right);
            up.Normalize();

            cameraToWorld = Matrix{
                {right.x,   right.y,   right.z,   0},
                {up.x,      up.y,      up.z,      0},
                {forward.x, forward.y, forward.z, 0},
                {origin.x,  origin.y,  origin.z,  1}
            };

            return cameraToWorld;
        }

        void Update(Timer* pTimer)
        {
            const float deltaTime = pTimer->GetElapsed();

            // Keyboard Input
            const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

            float movementSpeed = baseMovementSpeed;
            if (pKeyboardState[SDL_SCANCODE_LSHIFT])
                movementSpeed *= boostMultiplier;

            if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
                origin += forward * movementSpeed * deltaTime;

            if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
                origin -= forward * movementSpeed * deltaTime;

            if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
                origin += right * movementSpeed * deltaTime;

            if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
                origin -= right * movementSpeed * deltaTime;

            // Mouse Input
            SDL_GetRelativeMouseState(&mouseX, &mouseY);
            uint32_t mouseState = SDL_GetMouseState(nullptr, nullptr);

            bool lmb = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
            bool rmb = mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);

            if (rmb && !lmb)
            {
                // RMB: Rotate camera
                totalYaw += -mouseX * rotationSpeed;
                totalPitch += mouseY * rotationSpeed;
            }
            else if (lmb && !rmb)
            {
                // LMB: Move forward/back (mouse Y), Rotate yaw (mouse X)
                origin += forward * (mouseY * movementSpeed * 0.1f * deltaTime);
                totalYaw += -mouseX * rotationSpeed;
            }
            else if (lmb && rmb)
            {
                // LMB + RMB: Move up/down in world
                origin += Vector3::UnitY * (-mouseY * movementSpeed * 0.1f * deltaTime);
            }

            currMouseX = mouseX;
            currMouseY = mouseY;
        }
    };
}

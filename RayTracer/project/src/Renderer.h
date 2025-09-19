#pragma once

#include <cstdint>
#include "Matrix.h"
#include "Maths.h"
#include "Material.h"




struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		void RenderPixel(const Scene* pScene, const  uint32_t pixelIndex, const  float fov, const float aspectRatio, const Matrix& cameraToWorld, const Vector3& cameraOrigin, const std::vector<dae::Material*>&, const std::vector<dae::Light>&) const;
		bool SaveBufferToImage() const;

		void ToggleShadow();
		void CycleLightingMode();
	

	private:

		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		bool m_ShadowsEnabled{ true };
	};
}

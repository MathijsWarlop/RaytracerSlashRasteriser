#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

#include "Utils.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		void VertexTransformationWorldToViewSpace(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;

		void VertexTransformationWorldToProjectionSpace(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;

		void WorldViewProjectionMatrix(Mesh& mesh_in) const;
		ColorRGB PixelShading1(const Vertex_Out& out);
		
		void WorldViewProjectionMatrix2(Mesh& mesh_in) const;

		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out, const Matrix& worldMatrix) const;

		void Render_W1_Part1() ;
		void Render_W1_Part2();
		void Render_W1_Part3() ;
		void Render_W1_Part4() ;
		void Render_W1_Part5() ;

		void Render_W2_Part1();
		void Render_W2_Part2();
		void Render_W2_Part3();
		void Render_W2_Part4();
		void Render_W2_Part5();

		void Render_W3_Part1();
		void Render_W3_Part2();
		void Render_W3_Part3();
		void Render_W3_Part4();
		void Final();

		void RasterizeTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, std::vector<float>& depthBuffer);
		void RasterizeTriangleWithTexture(const Vertex& v0, const Vertex& v1, const Vertex& v2, Texture* texture, std::vector<float>& depthBuffer);

	
		float remap(float value, float fromLow, float fromHigh);
		ColorRGB PixelShading(const Vertex_Out& v) ;

		ColorRGB ApplySpecularLighting(const Vertex_Out& out, const ColorRGB& diffuse, const Vector3& normal, float lightIntensity);

		void ToggleFinalColorAndBuffer();
		void ToggleRotation();
		void ToggleNormal();
		void ToggleLightingMode();
		
		
		
	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		const Vector3 m_LightDirection = { .577f,-.577f,.577f };
		const Vector3 m_InvLightDirection = { -.577f,.577f,-.577f };

		Texture* m_ptexture;
		Texture* m_NormalMap;
		Texture* m_SpecularMap;
		Texture* m_GlossMap;
		//float* m_pDepthBufferPixels{};

		Camera m_Camera{  };
		float m_TurningAngle{};
		bool m_Rotating = true;
		float m_Shininess{25.f};
		float m_Yaw{};
		bool m_Buffer = false;
		bool m_Normal = true;

		int m_Width{};
		int m_Height{};

		float m_Kd{ 7.f };
		float m_Ks{ 0.5f };

		std::vector<Mesh> meshes_world
		{
			Mesh{
				{
					// Vertices with positions and UV coordinates
					//Vertex{{-3, 3, -2}, {}, {0, 0}},
					//Vertex{{0, 3, -2}, {}, {0.5f, 0}},
					//Vertex{{3, 3, -2}, {}, {1, 0}},
					//Vertex{{-3, 0, -2}, {}, {0, 0.5f}},
					//Vertex{{0, 0, -2}, {}, {0.5f, 0.5f}},
					//Vertex{{3, 0, -2}, {}, {1, 0.5f}},
					//Vertex{{-3, -3, -2}, {}, {0, 1}},
					//Vertex{{0, -3, -2}, {}, {0.5f, 1}},
					//Vertex{{3, -3, -2}, {}, {1, 1}}
				},
				{
					//3, 0, 4, 1, 5, 2,      // Triangle List indices
					//2, 6,
					//6, 3, 7, 4, 8, 5
				},
				PrimitiveTopology::TriangleList
			}
		};

		enum class LightingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined
		};
		LightingMode m_LightingMode{ LightingMode::Combined};
	};
}

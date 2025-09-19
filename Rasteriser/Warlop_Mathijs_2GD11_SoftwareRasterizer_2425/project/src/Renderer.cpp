//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>

#include "Camera.h"
#include <algorithm>
#include "BRDFs.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//for (auto& mesh : meshes_world)
	//{
	//	
	//}
	

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	//m_Camera
	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(m_Width, m_Height, { 45.f }, { 0.f,5.f,-64.f } );

	m_Camera.CalculateViewMatrix();

	//m_ptexture = Texture::LoadFromFile("resources/uv_grid_2.png");
	m_ptexture = Texture::LoadFromFile("resources/vehicle_diffuse.png");
	if (!m_ptexture)
	{
		SDL_Log("Failed to load texture");
		return;
	}
	m_NormalMap = Texture::LoadFromFile("resources/vehicle_normal.png");
	if (!m_NormalMap)
	{
		SDL_Log("Failed to load texture");
		return;
	}
	m_GlossMap = Texture::LoadFromFile("resources/vehicle_gloss.png");
	if (!m_GlossMap)
	{
		SDL_Log("Failed to load texture");
		return;
	}
	m_SpecularMap = Texture::LoadFromFile("resources/vehicle_specular.png");
	if (!m_SpecularMap)
	{
		SDL_Log("Failed to load texture");
		return;
	}
	for (auto& mesh : meshes_world)
	{
		Utils::ParseOBJ("resources/vehicle.obj", mesh.vertices, mesh.indices, true);
	}
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
	delete m_ptexture;
}

void Renderer::Update(Timer* pTimer)
{
	//m_TurningAngle% 360;
	if (m_Rotating)
	{
		m_Yaw += PI / 4 * pTimer->GetElapsed();
		meshes_world[0].worldMatrix = Matrix::CreateRotationY(m_Yaw) * Matrix::CreateTranslation(0.f, 0.f, 0.f);
	}
	
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC

	//Render_W1_Part1(); //1. Rasterizer Stage Only
	//Render_W1_Part2(); //2. Projection Stage(Camera)
	//Render_W1_Part3(); //3. Barycentric Coordinates
	//Render_W1_Part4(); //4. Depth Buffer + BoundingBox Optimization, but first loop pixels then triangles
	//Render_W1_Part5(); //5. BoundingBox Optimization  but first loop triangles then pixels (faster)

	//Render_W2_Part1(); //triangle list
	//Render_W2_Part2(); //fixing black seams
	//Render_W2_Part3(); //triangle strip
	//Render_W2_Part4(); //triangle tests
	//Render_W2_Part5(); //uv

	//Render_W3_Part1();
	//Render_W3_Part2();
	//Render_W3_Part3(); // render object
	//Render_W3_Part4();

	Final();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}



void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out, const Matrix& worldMatrix) const
{
	vertices_out.clear();
	vertices_out.reserve(vertices_in.size());
	Matrix worldViewProjectionMatrix = worldMatrix * m_Camera.invViewMatrix * m_Camera.projectionMatrix;
	for (int vertexIdx{ 0 }; vertexIdx < vertices_in.size(); ++vertexIdx)
	{
		Vertex_Out outVertex{};
		Vector4 outPoint = worldViewProjectionMatrix.TransformPoint(vertices_in[vertexIdx].position.ToPoint4());
	
		outPoint.x = (outPoint.x / outPoint.w);
		outPoint.y = (outPoint.y / outPoint.w);
		outPoint.z /= outPoint.w;

		// Map the position to screen space
		outPoint.x = (outPoint.x + 1) / 2 * m_Width;
		outPoint.y = (1 - outPoint.y) / 2 * m_Height;

		outVertex.position = outPoint;
		outVertex.color = vertices_in[vertexIdx].color;
		outVertex.uv = vertices_in[vertexIdx].uv;
		outVertex.normal = worldMatrix.TransformVector(vertices_in[vertexIdx].normal);
		outVertex.tangent = worldMatrix.TransformVector(vertices_in[vertexIdx].tangent);
	
		vertices_out.emplace_back(outVertex);
	}
	
}


bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void Renderer::Final()
{
	// Render Meshes in world
	for (auto& mesh : meshes_world)
	{
		// Apply the world-view-projection matrix
		VertexTransformationFunction(mesh.vertices, mesh.vertices_out, mesh.worldMatrix); // worldViewProjectionMatrix = worldmatrix * viewMatrix * projectionMatrix

		// Initialize the depth buffer and back buffer
		std::vector<float> depthBuffer(m_Width * m_Height, std::numeric_limits<float>::infinity());
		const int totalPixels = m_Width * m_Height;
		ColorRGB backgroundColor = { 0.5f, 0.5f, 0.5f };

		// Initialize background color
		for (int i = 0; i < totalPixels; ++i)
		{
			const int px = i % m_Width, py = i / m_Width;
			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(backgroundColor.r * 255),
				static_cast<uint8_t>(backgroundColor.g * 255),
				static_cast<uint8_t>(backgroundColor.b * 255));
		}

		// Process triangles as TriangleList
		for (size_t i = 0; i < mesh.indices.size(); i += 3) // Each triangle
		{
			int idx0 = mesh.indices[i], idx1 = mesh.indices[i + 1], idx2 = mesh.indices[i + 2];

			const Vertex_Out& v0 = mesh.vertices_out[idx0];
			const Vertex_Out& v1 = mesh.vertices_out[idx1];
			const Vertex_Out& v2 = mesh.vertices_out[idx2];

			// Check if any vertex is outside the screen bounds
			if (v0.position.x < 0 || v0.position.x >= m_Width || v0.position.y < 0 || v0.position.y >= m_Height ||
				v1.position.x < 0 || v1.position.x >= m_Width || v1.position.y < 0 || v1.position.y >= m_Height ||
				v2.position.x < 0 || v2.position.x >= m_Width || v2.position.y < 0 || v2.position.y >= m_Height)
			{
				continue; // Skip this triangle if any vertex is outside the screen bounds
			}

			// Calculate the bounding box of the triangle
			int minX = std::min({ v0.position.x, v1.position.x, v2.position.x });
			int maxX = std::max({ v0.position.x, v1.position.x, v2.position.x });
			int minY = std::min({ v0.position.y, v1.position.y, v2.position.y });
			int maxY = std::max({ v0.position.y, v1.position.y, v2.position.y });

			// Ensure the bounding box is within screen bounds
			minX = std::max(0, std::min(minX, m_Width - 1));
			maxX = std::max(0, std::min(maxX, m_Width - 1));
			minY = std::max(0, std::min(minY, m_Height - 1));
			maxY = std::max(0, std::min(maxY, m_Height - 1));

			// Loop through the bounding box and check for pixels inside the triangle
			for (int y = minY; y <= maxY; ++y) // Every pixel
			{
				for (int x = minX; x <= maxX; ++x)
				{
					// Barycentric coordinates (area-based interpolation)
					float denom = (v1.position.y - v2.position.y) * (v0.position.x - v2.position.x) + (v2.position.x - v1.position.x) * (v0.position.y - v2.position.y);
					if (fabs(denom) < 1e-6f) { continue; }  // Skip degenerate triangles

					// Calculate lambda0, lambda1, lambda2 (Barycentric coordinates)
					float lambda0 = ((v1.position.y - v2.position.y) * (x - v2.position.x) + (v2.position.x - v1.position.x) * (y - v2.position.y)) / denom;
					float lambda1 = ((v2.position.y - v0.position.y) * (x - v2.position.x) + (v0.position.x - v2.position.x) * (y - v2.position.y)) / denom;
					float lambda2 = 1.0f - lambda0 - lambda1;

					// Check if the pixel is inside the triangle (lambda0, lambda1, lambda2 >= 0)
					if (lambda0 >= 0 && lambda1 >= 0 && lambda2 >= 0)
					{
						// Interpolate the depth using the inverse depth interpolation formula
						float depth0 = v0.position.w;
						float depth1 = v1.position.w;
						float depth2 = v2.position.w;

						// Apply the inverse depth interpolation formula
						float interpolatedDepth = 1.0f / ((lambda0 / depth0) + (lambda1 / depth1) + (lambda2 / depth2));

						// Perspective-correct UV interpolation
						Vector2 uv = ((v0.uv / v0.position.w) * lambda0 +
							(v1.uv / v1.position.w) * lambda1 +
							(v2.uv / v2.position.w) * lambda2) * interpolatedDepth;

						// Clamp UVs to [0, 1] range
						uv.x = std::clamp(uv.x, 0.0f, 1.f);
						uv.y = std::clamp(uv.y, 0.0f, 1.f);

						// Get the pixel index for the current pixel (x, y)
						int pixelIndex = y * m_Width + x;

						// Perform the depth test (check if the interpolated depth is closer)
						if (interpolatedDepth < depthBuffer[pixelIndex])
						{
							// Update the depth buffer with the new interpolated depth
							depthBuffer[pixelIndex] = interpolatedDepth;

							// Toggle between normal rendering and depth visualization
							if (!m_Buffer)
							{
								// Normal pixel shading (apply lighting, textures, etc.)
								Vertex_Out pixelOut;

								pixelOut.color = m_ptexture->Sample(uv);
								pixelOut.position = Vector4{ static_cast<float>(x), static_cast<float>(y), interpolatedDepth, 1.0f };
								pixelOut.uv = Vector2{ lambda0 * v0.uv.x + lambda1 * v1.uv.x + lambda2 * v2.uv.x, lambda0 * v0.uv.y + lambda1 * v1.uv.y + lambda2 * v2.uv.y };
								pixelOut.normal = lambda0 * v0.normal + lambda1 * v1.normal + lambda2 * v2.normal; // Interpolate normals
								pixelOut.tangent = lambda0 * v0.tangent + lambda1 * v1.tangent + lambda2 * v2.tangent; // Interpolate tangents
								pixelOut.viewDirection = (m_Camera.origin - pixelOut.position).Normalized();


								// Call PixelShading to get the color
								ColorRGB finalColor = PixelShading(pixelOut);

								finalColor.r = std::clamp(finalColor.r, 0.0f, 1.f);
								finalColor.g = std::clamp(finalColor.g, 0.0f, 1.f);
								finalColor.b = std::clamp(finalColor.b, 0.0f, 1.f);
								// Update the pixel color on the back buffer
								m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(finalColor.r * 255),
									static_cast<uint8_t>(finalColor.g * 255),
									static_cast<uint8_t>(finalColor.b * 255));
							}
							else
							{
								// OPTIONAL: Visualize the depth by encoding depth values as a color
								float depthNormalized = remap(interpolatedDepth, 1.f, 100.f);

								// Update the pixel color on the back buffer with the depth color
								m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(depthNormalized * 255),  // Red channel
									static_cast<uint8_t>(depthNormalized * 255),  // Green channel
									static_cast<uint8_t>(depthNormalized * 255)); // Blue channel
							}
						}
					}
				}
			}
		}
	}
}

float Renderer::remap(float value, float fromLow, float fromHigh)
{
	// Normalize the value to the range [0, 1]
	float normalized = (value - fromLow) / (fromHigh - fromLow);

	// Ensure the value stays within [0, 1]
	normalized = std::clamp(normalized, 0.0f, 1.0f);

	return normalized; // Return the normalized value
}

ColorRGB Renderer::PixelShading(const Vertex_Out& v) 
{
	Vector3 normalToUse{ v.normal };
	
	//m_ptexture
	if (m_Normal)
	{
		const Vector3 binormal{ Vector3::Cross(v.normal, v.tangent) };
		const Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3::Zero };
		const ColorRGB sampledNormal{ m_NormalMap->Sample(v.uv) };
		Vector3 normal{ sampledNormal.r, sampledNormal.g, sampledNormal.b };
		normal /= 255.f;
		normal = 2.f * normal - Vector3{ 1.f, 1.f, 1.f };

		Vector3 transformedNormal{ tangentSpaceAxis.TransformVector(normal) };
		transformedNormal.Normalize();
		normalToUse = transformedNormal;
	}

	switch (m_LightingMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
	{
		const float observedArea{ std::max(Vector3::Dot(normalToUse, m_InvLightDirection), 0.f) };
		return { colors::White * observedArea };
		break;
	}
	case dae::Renderer::LightingMode::Diffuse:
	{
		ColorRGB Color = ((m_Kd * v.color) / PI) / 255.f;

		return{ Color };
		break;
	}
	case dae::Renderer::LightingMode::Specular:
	{
		Vector3 viewDirection = (m_Camera.origin - v.position).Normalized();
		ColorRGB specularColor = m_SpecularMap->Sample(v.uv);
		Vector3 reflectDirection = 2.0f * Vector3::Dot(normalToUse, m_InvLightDirection) * normalToUse + m_LightDirection;
		float rDotV = std::max(0.0f, Vector3::Dot(reflectDirection, -v.viewDirection.Normalized()));
		float specular = std::pow(rDotV, m_Shininess);

		ColorRGB specularFinal = specularColor * specular;

		//std::cout<< v.viewDirection.x << "   " << v.viewDirection.y  << "   " << v.viewDirection.z << std::endl;

		return{ specularFinal / 255.f };
		break;
	}
	case dae::Renderer::LightingMode::Combined:
	{
		//Lambert
		const ColorRGB lambertDiffuse = (m_Kd * v.color) / M_PI;
		const float observedArea = std::max(Vector3::Dot(normalToUse, m_InvLightDirection), 0.f);

		//specular
		Vector3 viewDirection = (m_Camera.origin - v.position).Normalized();
		ColorRGB specularColor = m_SpecularMap->Sample(v.uv);
		Vector3 reflectDirection = 2.0f * Vector3::Dot(normalToUse, m_InvLightDirection) * normalToUse + m_LightDirection;
		float rDotV = std::max(0.0f, Vector3::Dot(reflectDirection, -v.viewDirection.Normalized()));
		float specular = std::pow(rDotV, m_Shininess);

		ColorRGB specularFinal = specularColor * specular;

		return{ ((lambertDiffuse * observedArea) + specularFinal) / 255.f };
		break;
	}
	}
}

void Renderer::ToggleFinalColorAndBuffer()
{
	m_Buffer = !m_Buffer;
	
}
void Renderer::ToggleRotation()
{
	m_Rotating = !m_Rotating;
}
void Renderer::ToggleNormal()
{
	m_Normal = !m_Normal;
	m_Buffer = false;
}
void Renderer::ToggleLightingMode()
{
	m_Buffer = false;


	// Cycle through lighting modes
	switch (m_LightingMode)
	{
	case LightingMode::ObservedArea:
		m_LightingMode = LightingMode::Diffuse;
		break;
	case LightingMode::Diffuse:
		m_LightingMode = LightingMode::Specular;
		break;
	case LightingMode::Specular:
		m_LightingMode = LightingMode::Combined;
		break;
	case LightingMode::Combined:
		m_LightingMode = LightingMode::ObservedArea;
		break;
	default:
		m_LightingMode = LightingMode::ObservedArea;
		break;
	}

	//std::cout << "Current Lighting Mode: " << static_cast<int>(m_LightingMode) << std::endl;  // Optional: to print the current mode
}
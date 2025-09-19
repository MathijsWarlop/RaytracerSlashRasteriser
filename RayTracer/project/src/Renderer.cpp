//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <iostream>
#include <execution>


#define PARALLEL_EXECUTION

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}
void Renderer::Render(Scene* pScene) const
{
    Camera& camera = pScene->GetCamera();

    // Cache necessary values outside the loop
    const Matrix cameraToWorld = camera.CalculateCameraToWorld();
    const float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    const float fov = tan(camera.fovAngle / 2);
    const auto& materials = pScene->GetMaterials();
    const auto& lights = pScene->GetLights();

    const uint32_t amountOfPixels = m_Width * m_Height;

#if defined(PARALLEL_EXECUTION)


    std::vector<uint32_t> pixelIndeces(amountOfPixels);
    std::iota(pixelIndeces.begin(), pixelIndeces.end(), 0); // Fill with indices 0, 1, ..., amountOfPixels-1
    
    std::for_each(std::execution::par, pixelIndeces.begin(), pixelIndeces.end(),
        [&](const uint32_t& pixelIndex)
        {
            RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin, materials, lights);
        });
#else
    for (uint32_t pixelIndex = 0; pixelIndex < amountOfPixels; ++pixelIndex)
    {
        RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin, materials, lights);
    }
#endif

    SDL_UpdateWindowSurface(m_pWindow);
}



void Renderer::RenderPixel(const Scene* pScene, const uint32_t pixelIndex, const float fov, const float aspectRatio, const Matrix& cameraToWorld, const Vector3& cameraOrigin, const std::vector<dae::Material*>& materials, const std::vector<dae::Light>& lights) const
{
    const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

    // Calculate ray direction with FOV and aspect ratio adjustments
    const float Cx = ((2 * ((px + 0.5f) / float(m_Width))) - 1) * aspectRatio * fov;
    const float Cy = (1 - 2 * ((py + 0.5f) / float(m_Height))) * fov;

    Vector3 rayDirection{ Cx, Cy, 1 };
    rayDirection = cameraToWorld.TransformVector(rayDirection).Normalized();

    Ray viewRay{ cameraOrigin, rayDirection };
    ColorRGB finalColor{};
    HitRecord closestHit{};

    // Find the closest hit for the view ray
    pScene->GetClosestHit(viewRay, closestHit);

    // If there's a hit, calculate lighting
    if (closestHit.didHit)
    {
        const Vector3& normalizedHitNormal = closestHit.normal.Normalized();
        Vector3 closestHitLocation = closestHit.origin + 0.001f * normalizedHitNormal;
    
        for (const auto& light : lights)
        {
            if (light.type != LightType::Point) continue;
    
            Vector3 closestHitDirectionToLight = light.origin - closestHitLocation;
            float maxDistance = closestHitDirectionToLight.Magnitude();
            Vector3 normalizedDirectionToLight = closestHitDirectionToLight.Normalized();
    
            float NdotL = Vector3::Dot(normalizedDirectionToLight, normalizedHitNormal);
            if (NdotL <= 0) continue;
    
            Ray hitTowardsLightRay(closestHitLocation, normalizedDirectionToLight, 0.0001f, maxDistance);
            bool isInShadow = m_ShadowsEnabled && pScene->DoesHit(hitTowardsLightRay);
    
            if (!isInShadow)
            {
                switch (m_CurrentLightingMode) {
                case LightingMode::Combined:
                    finalColor += LightUtils::GetRadiance(light, closestHitLocation)
                        * materials[closestHit.materialIndex]->Shade(closestHit, normalizedDirectionToLight, -rayDirection)
                        * NdotL;
                    break;
                case LightingMode::ObservedArea:
                    finalColor += ColorRGB{ 1, 1, 1 } *NdotL;
                    break;
                case LightingMode::Radiance:
                    finalColor += LightUtils::GetRadiance(light, closestHitLocation);
                    break;
                case LightingMode::BRDF:
                    finalColor += materials[closestHit.materialIndex]->Shade(closestHit, normalizedDirectionToLight, -rayDirection);
                    break;
                }
            }
        }
    }

    finalColor.MaxToOne();

    // Update Color in Buffer
    m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
        static_cast<uint8_t>(finalColor.r * 255.f),
        static_cast<uint8_t>(finalColor.g * 255.f),
        static_cast<uint8_t>(finalColor.b * 255.f));
}
bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
void Renderer::ToggleShadow()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}
void Renderer::CycleLightingMode()
{
	if (m_CurrentLightingMode == LightingMode::ObservedArea)
	{
		m_CurrentLightingMode = LightingMode::Radiance;
	}
	else if (m_CurrentLightingMode == LightingMode::Radiance)
	{
		m_CurrentLightingMode = LightingMode::BRDF;
	}
	else if (m_CurrentLightingMode == LightingMode::BRDF)
	{
		m_CurrentLightingMode = LightingMode::Combined;
	}
	else { m_CurrentLightingMode = LightingMode::ObservedArea; }
}

//First vergion
//void Renderer::RenderPixel(const Scene* pScene, const uint32_t pixelIndex, const float fov, const float aspectRatio, const Matrix& cameraToWorld, const Vector3& cameraOrigin, const std::vector<dae::Material*>& material, const std::vector<dae::Light>& lights) const
//{
//	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };
//
//	const float Cx = ((2 * ((px + 0.5) / float(m_Width))) - 1) * aspectRatio * fov; //WITH FOV
//	const float Cy = (1 - 2 * ((py + 0.5) / float(m_Height))) * fov;
//
//	Vector3 rayDirection{ Cx,Cy,1 };
//	rayDirection = cameraToWorld.TransformVector(rayDirection);
//	rayDirection.Normalize();
//
//	//Ray viewRay{ {}, rayDirection };// from origin
//	Ray viewRay{ cameraOrigin, rayDirection };// from camera
//	ColorRGB finalColor{};
//	HitRecord closestHit{};
//
//	pScene->GetClosestHit(viewRay, closestHit);
//
//	if (closestHit.didHit)
//	{
//
//		for (size_t LightIndex{ 0 }; LightIndex < lights.size(); LightIndex++)
//		{
//			Vector3 closestHitLocation = closestHit.origin + 0.001 * closestHit.normal;
//			Vector3 closestHitDirectionToLight = lights[LightIndex].origin - closestHitLocation;
//			float max = closestHitDirectionToLight.Magnitude();
//			Ray HitTowardsLightRay = Ray(closestHitLocation, closestHitDirectionToLight.Normalized());
//			HitTowardsLightRay.min = 0.0001;
//			HitTowardsLightRay.max = max;
//
//			if (lights[LightIndex].type == LightType::Point)
//			{
//
//				if (Vector3::Dot(closestHitDirectionToLight.Normalized(), closestHit.normal.Normalized()) > 0)
//				{
//					if (m_ShadowsEnabled)
//					{
//						if (!pScene->DoesHit(HitTowardsLightRay))
//						{
//							switch (m_CurrentLightingMode) {
//							case(LightingMode::Combined):
//								finalColor += LightUtils::GetRadiance(lights[LightIndex], closestHitLocation) * material[closestHit.materialIndex]->Shade(closestHit, closestHitDirectionToLight.Normalized(), -rayDirection.Normalized()) * Vector3::Dot(closestHitDirectionToLight.Normalized(), closestHit.normal);
//								break;
//							case(LightingMode::ObservedArea):
//								finalColor += ColorRGB{ 1,1,1 } *Vector3::Dot(closestHitDirectionToLight.Normalized(), closestHit.normal);
//								break;
//							case(LightingMode::Radiance):
//								finalColor += LightUtils::GetRadiance(lights[LightIndex], closestHitLocation);
//								break;
//							case(LightingMode::BRDF):
//								ColorRGB Mat = material[closestHit.materialIndex]->Shade(closestHit, closestHitDirectionToLight.Normalized(), -rayDirection.Normalized());
//								finalColor += Mat;
//								break;
//							}
//						}
//					}
//					else
//					{
//						switch (m_CurrentLightingMode) {
//						case(LightingMode::Combined):
//							finalColor += LightUtils::GetRadiance(lights[LightIndex], closestHitLocation) * material[closestHit.materialIndex]->Shade(closestHit, closestHitDirectionToLight.Normalized(), -rayDirection.Normalized()) * Vector3::Dot(closestHitDirectionToLight.Normalized(), closestHit.normal);
//							break;
//						case(LightingMode::ObservedArea):
//							finalColor += ColorRGB{ 1,1,1 } *Vector3::Dot(closestHitDirectionToLight.Normalized(), closestHit.normal);
//							break;
//						case(LightingMode::Radiance):
//							finalColor += LightUtils::GetRadiance(lights[LightIndex], closestHitLocation);
//							break;
//						case(LightingMode::BRDF):
//							ColorRGB Mat = material[closestHit.materialIndex]->Shade(closestHit, closestHitDirectionToLight.Normalized(), -rayDirection.Normalized());
//							finalColor += Mat;
//							break;
//						}
//					}
//				}
//			}
//		}
//	}
//	finalColor.MaxToOne();
//	//Update Color in Buffer
//
//
//	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
//		static_cast<uint8_t>(finalColor.r * 255.f),
//		static_cast<uint8_t>(finalColor.g * 255.f),
//		static_cast<uint8_t>(finalColor.b * 255.f));
//}

//void Renderer::Render(Scene* pScene) const
//{
//	Camera& camera = pScene->GetCamera();
//
//	const Matrix cameraToWorld = camera.CalculateCameraToWorld();
//
//	float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
//
//	float fov = tan(camera.fovAngle / 2); //in Radians (PI/4) = 45 degrees
//
//	const std::vector<dae::Material*>& materials{ pScene->GetMaterials() };
//	const std::vector<dae::Light>& lights{ pScene->GetLights() };
//
//#if defined(PARALLEL_EXECUTION)
//
//	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
//	std::vector<uint32_t> pixelIndeces{};
//
//	pixelIndeces.reserve(amountOfPixels);
//	for (uint32_t index{}; index < amountOfPixels; ++index) pixelIndeces.emplace_back(index);
//
//	std::for_each(std::execution::par, pixelIndeces.begin(), pixelIndeces.end(), [&](int i)
//		{
//			RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin, materials, lights);
//		});
//
//#else
//	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
//
//	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; pixelIndex++)
//	{
//		RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin, materials, lights);
//	}
//
//#endif
//
//	SDL_UpdateWindowSurface(m_pWindow);
//}
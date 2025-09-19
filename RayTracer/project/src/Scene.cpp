#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		//todo W1
		HitRecord FinalClosestHit;

		for (size_t sphereIndex{ 0 }; sphereIndex < m_SphereGeometries.size(); sphereIndex++)
		{
			GeometryUtils::HitTest_Sphere(m_SphereGeometries[sphereIndex], ray, closestHit);
			if (closestHit.t < FinalClosestHit.t)
			{
				FinalClosestHit = closestHit;
				
			}
		}
		for (size_t planeIndex{ 0 }; planeIndex < m_PlaneGeometries.size(); planeIndex++)
		{
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[planeIndex], ray, closestHit);
			if (closestHit.t < FinalClosestHit.t)
			{
				FinalClosestHit = closestHit;
			}
		}
		//for (size_t TriangleMeshIndex{ 0 }; TriangleMeshIndex < m_TriangleMeshGeometries.size(); TriangleMeshIndex++)
		//{
		//	GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[TriangleMeshIndex], ray, closestHit);
		//	if (closestHit.t < FinalClosestHit.t)
		//	{
		//		FinalClosestHit = closestHit;
		//	}
		//	
		//}

		for (size_t TriangleMeshIndex{ 0 }; TriangleMeshIndex < m_TriangleMeshGeometries.size(); TriangleMeshIndex++)
		{
			if (GeometryUtils::SlabTest_TriangleMesh(m_TriangleMeshGeometries[TriangleMeshIndex], ray))
			{
				for (int indicesIndex{}; indicesIndex < m_TriangleMeshGeometries[TriangleMeshIndex].indices.size(); indicesIndex += 3)
				{

					Vector3 pos1 = m_TriangleMeshGeometries[TriangleMeshIndex].transformedPositions[m_TriangleMeshGeometries[TriangleMeshIndex].indices[indicesIndex]];
					Vector3 pos2 = m_TriangleMeshGeometries[TriangleMeshIndex].transformedPositions[m_TriangleMeshGeometries[TriangleMeshIndex].indices[indicesIndex + 1]];
					Vector3 pos3 = m_TriangleMeshGeometries[TriangleMeshIndex].transformedPositions[m_TriangleMeshGeometries[TriangleMeshIndex].indices[indicesIndex + 2]];

					//dae::Triangle();
					dae::Triangle triangle = Triangle{ pos1, pos2, pos3 };
					triangle.cullMode = m_TriangleMeshGeometries[TriangleMeshIndex].cullMode;
					triangle.materialIndex = m_TriangleMeshGeometries[TriangleMeshIndex].materialIndex;

					GeometryUtils::HitTest_Triangle(triangle, ray, closestHit);
					if (closestHit.t < FinalClosestHit.t)
					{
						FinalClosestHit = closestHit;
					}
				}
			}
			else
			{
				//std::cout << "flase";
			}
		}


		closestHit = FinalClosestHit;
		//-------
		//throw std::runtime_error("Not Implemented Yet");
	}
	
bool Scene::DoesHit(const Ray& ray) const
{
    // First check for intersections with spheres (early return on hit)
    for (const  dae::Sphere& sphere : m_SphereGeometries)
    {
        if (GeometryUtils::HitTest_Sphere(sphere, ray))
        {
            return true; // Ray hits a sphere
        }
    }

    // Check for intersection with planes
    for (const dae::Plane& plane : m_PlaneGeometries)
    {
        if (GeometryUtils::HitTest_Plane(plane, ray))
        {
            return true; // Ray hits a plane
        }
    }

    // If no intersection, check for intersections with triangle meshes using BVH or acceleration structures
    for (const dae::TriangleMesh& mesh : m_TriangleMeshGeometries)
    {
        if (GeometryUtils::SlabTest_TriangleMesh(mesh, ray))
        {
            // Check each triangle only if the slab test passed
            for (size_t i = 0; i < mesh.indices.size(); i += 3)
            {
                const Vector3& pos1 = mesh.transformedPositions[mesh.indices[i]];
                const Vector3& pos2 = mesh.transformedPositions[mesh.indices[i + 1]];
                const Vector3& pos3 = mesh.transformedPositions[mesh.indices[i + 2]];

                Triangle triangle{ pos1, pos2, pos3 };
                triangle.cullMode = mesh.cullMode;
                triangle.materialIndex = mesh.materialIndex;

                if (GeometryUtils::HitTest_Triangle(triangle, ray))
                {
                    return true; // Ray hits a triangle
                }
            }
        }
    }

    return false; // No hit
}


	

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma region SCENE W2
	void Scene_W2::Initialize()
	{

		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor(colors::Blue));

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor(colors::Yellow));
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor(colors::Green));
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor(colors::Magenta));

		//Planes
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, 0.75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, 0.75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, 0.75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, 0.75f, matId_Solid_Blue);

		//Lights
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);

	}
#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));

		//Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matLambert_GrayBlue);

		AddSphere({ -1.75f, 1.f, 0.f }, 0.75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, 0.75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, 0.75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, 0.75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, 0.75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, 0.75f, matCT_GraySmoothPlastic);

		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

#pragma region SCENE W3
	void Scene_W3_TestScene1::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		const auto matLambert_Red = AddMaterial(new Material_Lambert(colors::Red, 1.f));
		const auto matLambert_Blue = AddMaterial(new Material_Lambert(colors::Blue,1.f));
		const auto matLambert_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));
		const auto matLambertPhong = AddMaterial(new Material_LambertPhong(colors::Blue, 1.f, 1.f, 60.f));

		AddSphere({ -.75f, 1.f, 0.f }, 1.f, matLambert_Red);
		AddSphere({ 0.75f, 1.f, 0.f }, 1.f, matLambertPhong);

		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambert_Yellow);

		AddPointLight({ 0.f, 2.5f, -5.f }, 25.f, colors::White);
		AddPointLight({ 0.f, 5.f, 5.f }, 25.f,colors::White);
		
	}
#pragma region SCENE W3
	void Scene_W3_TestScene2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.f));

		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f },matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);
		
		
		

		const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f,3.f));
		const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f,15.f));
		const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f,50.f));

	
		AddSphere({ -1.75f, 1.f, 0.f }, 0.75f, matLambertPhong1);
		AddSphere({ 0.f, 1.f, 0.f }, 0.75f, matLambertPhong2);
		AddSphere({ 1.75f, 1.f, 0.f }, 0.75f, matLambertPhong3);
		AddSphere({ -1.75f, 3.f, 0.f }, 0.75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, 0.75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, 0.75f, matCT_GraySmoothPlastic);
		
;

		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
#pragma region SCENE W4
	void Scene_W4_TestScene1::Initialize()
	{

		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		// Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));

		// Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		// Triangle (temp)
		auto triangle = Triangle{ {-.75f, .5f, .0f}, {-.75f, 2.f, .0f}, { .75f, .5f, .0f} };
		triangle.cullMode = TriangleCullMode::NoCulling;
		triangle.materialIndex = matLambert_White;
		
		m_TriangleGeometries.emplace_back(triangle);

		// Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

	}
#pragma region SCENE W4
	void Scene_W4_TestScene2::Initialize()
	{

		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		// Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));

		// Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		// Triangle (temp)
		//auto triangle = Triangle{ {-.75f, .5f, .0f}, {-.75f, 2.f, .0f}, { .75f, .5f, .0f} };
		//triangle.cullMode = TriangleCullMode::NoCulling;
		//triangle.materialIndex = matLambert_White;

		//m_TriangleGeometries.emplace_back(triangle);

		//Triangle mesh
	
		const auto triangleMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		triangleMesh->positions = { {-.75f,-1.f,.0f},{-.75f,1.f,.0f} ,{.75f,1.f,1.f},{.75f,-1.f,0.f} };
		triangleMesh->indices = {
		0,1,2,
		0,2,3
		};

		triangleMesh->CalculateNormals();

		triangleMesh->Translate({0.f,1.5f,0.f});
		triangleMesh->RotateY(45);
		triangleMesh->Scale({1.f,1.f,1.f});

		triangleMesh->UpdateTransforms();

		// Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

	}
#pragma region SCENE W4
	void Scene_W4_TestScene3::Initialize()
	{

		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		// Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));

		// Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//pMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		//pMesh->positions = { 
		//	{-.75f,-1.f,.0f},
		//	{-.75f,1.f,.0f} ,
		//	{.75f,1.f,1.f},
		//	{.75f,-1.f,0.f} };
		//pMesh->indices = {
		//0,1,2,
		//0,2,3
		//};
		//pMesh->CalculateNormals();
		//pMesh->Translate({ 0.f,1.5f,0.f });
		////pMesh->RotateY(45);
		////pMesh->Scale({ 1.f,1.f,1.f });
		//pMesh->UpdateTransforms();

		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/simple_Cube.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);

		pMesh->Scale({ .7f,.7f,.7f });
		pMesh->Translate({ 0.f,1.f,0.f });

		//normals are calculated inside ParseOBJ
		pMesh->UpdateTransforms();

		

		// Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

	}
	void Scene_W4_TestScene3::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		pMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
		pMesh->UpdateTransforms();
	}
	void Scene_W4_RefrenceScene::Initialize()
	{
		sceneName = "RefrenceScene";
		m_Camera.origin = { 0, 3, -9 };
		m_Camera.fovAngle = 45.f;

		// Materials
		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));

		// Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, 0.75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, 0.75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, 0.75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, 0.75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f	, 3.f, 0.f }, 0.75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, 0.75f, matCT_GraySmoothPlastic);

		//CW winding order!
		const Triangle baseTriangle = { Vector3(-0.75,1.5f,0.f),Vector3(0.75,0.f,0.f),Vector3(-0.75,0.f,0.f) };

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate({ -1.75f,4.5f,0.f });
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate({ 0.f,4.5f,0.f });
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate({ 1.75f,4.5f,0.f });
		m_Meshes[2]->UpdateTransforms();

		
		// Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

	}
	void Scene_W4_RefrenceScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		for (const auto m : m_Meshes)
		{
			m->RotateY(PI_DIV_2 * pTimer->GetTotal());
			m->UpdateAABB();
			m->UpdateTransforms();
		}

		
	}
	void Scene_W4_BunnyScene::Initialize()
	{
		sceneName = "BunnyScene";
		m_Camera.origin = { 0, 3, -9 };
		m_Camera.fovAngle = 45.f;

		// Materials
		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.0f));

		// Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//BUNNY MESH
		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);
		
		pMesh->Scale({ 2.f,2.f,2.f });
		pMesh->RotateY(PI);

		pMesh->UpdateAABB();
		
		//normals are calculated inside ParseOBJ
		pMesh->UpdateTransforms();


		// Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

	}
#pragma endregion
}



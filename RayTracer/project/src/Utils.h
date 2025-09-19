#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
{
    // Early exit if no hit is possible
    Vector3 sphereVector = ray.origin - sphere.origin;
    float A = Vector3::Dot(ray.direction, ray.direction);
    float B = Vector3::Dot(sphereVector, 2 * ray.direction);
    float C = Vector3::Dot(sphereVector, sphereVector) - sphere.radius * sphere.radius;
    float discriminant = B * B - 4 * A * C;

    if (discriminant < 0) return false; // No intersection

    // Calculate the intersection point
    float sqrtDisc = std::sqrt(discriminant);
    float t = (-B - sqrtDisc) / (2 * A);

    if (t < ray.min || t >= ray.max) return false;

    if (!ignoreHitRecord)
    {
        hitRecord.t = t;
        hitRecord.didHit = true;
        hitRecord.materialIndex = sphere.materialIndex;
        hitRecord.origin = ray.origin + t * ray.direction;
        hitRecord.normal = hitRecord.origin - sphere.origin;
        hitRecord.normal.Normalize();
    }

    return true;
}
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			
			const float t{ (Vector3::Dot(plane.origin - ray.origin , plane.normal) / Vector3::Dot(ray.direction,plane.normal)) };
			if (t < ray.min) return false;
			if (t >= ray.max) return false;
			if (ignoreHitRecord) return true;
			hitRecord.t = t;
			hitRecord.didHit = true;
			hitRecord.materialIndex = plane.materialIndex;
			hitRecord.origin = ray.origin + t* (ray.direction);
			hitRecord.normal = plane.normal;
			
			return true;		
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// Step 1: Check if the ray intersects the triangle's plane
			Vector3 edge1 = triangle.v1 - triangle.v0;
			Vector3 edge2 = triangle.v2 - triangle.v0;
			Vector3 normal = Vector3::Cross(edge1, edge2).Normalized();

			float faceDotRayDir = Vector3::Dot(normal, ray.direction);

			// Check for parallel ray
			if (AreEqual(faceDotRayDir, 0)) return false;

			// Cull mode check, simplified
			if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && faceDotRayDir < 0) return false;
			if (triangle.cullMode == TriangleCullMode::BackFaceCulling && faceDotRayDir > 0) return false;

			// Step 2: Calculate the point of intersection
			float t = Vector3::Dot(triangle.v0 - ray.origin, normal) / faceDotRayDir;

			// Ensure t is within the ray bounds
			if (t < ray.min || t >= ray.max) return false;

			// Calculate the hit point
			Vector3 hitPoint = ray.origin + t * ray.direction;

			// Step 3: Perform inside-outside test
			Vector3 c;

			Vector3 edge = triangle.v1 - triangle.v0;
			Vector3 vp = hitPoint - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(edge, vp), normal) < 0) return false;

			edge = triangle.v2 - triangle.v1;
			vp = hitPoint - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(edge, vp), normal) < 0) return false;

			edge = triangle.v0 - triangle.v2;
			vp = hitPoint - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(edge, vp), normal) < 0) return false;

			// Step 4: Fill the HitRecord if needed
			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = hitPoint;
				hitRecord.normal = normal;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}
			bool hit{ false };
			for (int indicesIndex{}; indicesIndex < mesh.indices.size(); indicesIndex += 3)
			{
				HitRecord temp;
				HitRecord FinalClosestHitTriangle;

				Vector3 pos1 = mesh.transformedPositions[mesh.indices[indicesIndex]];
				Vector3 pos2 = mesh.transformedPositions[mesh.indices[indicesIndex + 1]];
				Vector3 pos3 = mesh.transformedPositions[mesh.indices[indicesIndex + 2]];

				//dae::Triangle();
				auto triangle = Triangle{ pos1, pos2, pos3 };
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				if (GeometryUtils::HitTest_Triangle(triangle, ray));
				{
					hit = true;
					GeometryUtils::HitTest_Triangle(triangle, ray, temp);
					if (temp.t < hitRecord.t)
					{
						hitRecord = temp;

						
					}
				}

				
			}
			return true;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}

		
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			if (light.type == LightType::Directional) return -light.direction;
			return Vector3{ light.origin - origin }; return Vector3{ light.origin.x - origin.x,light.origin.y - origin.y,light.origin.z - origin.z };
			//---
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			// W/(m^2*sr)
			
			if (light.type == LightType::Point)
			{
				ColorRGB Clr = (light.color * (light.intensity / Vector3::Dot((light.origin - target), (light.origin - target))));
				return Clr;
			}
			else if (light.type == LightType::Directional)
			{
				return (light.color * light.intensity);
			}
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}

#pragma once
#include "Maths.h"
#include <iostream>
namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB rho = (kd * cd);
			ColorRGB color = rho / PI;
			return color;
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			ColorRGB rho = (kd * cd);
			
			return rho / PI;
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3
			Vector3 r = Vector3::Reflect(l, n);
			float cosa = Vector3::Dot(r, v);
			if (cosa < 0)cosa = 0;
			else if (cosa > 90)cosa = 90;
			float phongSpecularReflection = ks * (pow(cosa, exp)) ;


			//ColorRGB color = ColorRGB(1, 1, 1)* phongSpecularReflection;
			//std::cout << phongSpecularReflection << std::endl;
			//color.MaxToOne(); 
			return  ColorRGB(1, 1, 1) * phongSpecularReflection;;
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3
			auto Frgb = f0 + (ColorRGB{ 1.0f, 1.0f, 1.0f } - f0) * powf(1.0f - Vector3::Dot(h, v), 5);
			return Frgb;
			// 
			
			//---
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			
			float a = roughness * roughness;
			float aSquared = a * a;

			float N = aSquared / (PI * powf((Vector3::Dot(n, h) * Vector3::Dot(n, h)) * (aSquared - 1) + 1, 2));
			return N;
			//----
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			
			float k = powf(roughness * roughness + 1, 2) / 8.0f;

			float GGX = Vector3::Dot(n, v) / (Vector3::Dot(n, v) * (1 - k) + k);

			return GGX;
			//---
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//todo: W3
			float GSmith = GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
			return GSmith;
			//---
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

	}
}
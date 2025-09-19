#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
		
	}

    Texture::~Texture()
    {
        if (m_pSurface)
        {
            SDL_FreeSurface(m_pSurface);
            m_pSurface = nullptr;
        }
    }

    Texture* Texture::LoadFromFile(const std::string& path)
    {
        // Load SDL_Surface using IMG_Load
        SDL_Surface* pSurface = IMG_Load(path.c_str());
        if (!pSurface)
        {
            SDL_Log("Failed to load image: %s", IMG_GetError());
            return nullptr;
        }

        // Convert to RGBA8888 format if needed
        if (pSurface->format->format != SDL_PIXELFORMAT_RGBA8888)
        {
            SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_RGBA8888, 0);
            if (convertedSurface)
            {
                SDL_FreeSurface(pSurface);
                pSurface = convertedSurface;
            }
            else
            {
                SDL_Log("Failed to convert surface format: %s", SDL_GetError());
                SDL_FreeSurface(pSurface);
                return nullptr;
            }
        }

        return new Texture(pSurface);
    }

    ColorRGB Texture::Sample(const Vector2& uv) const
    {
        // Clamp the UV coordinates to the [0, 1] range
        float u = std::clamp(uv.x, 0.0f, 1.0f);
        float v = std::clamp(uv.y, 0.0f, 1.0f);

        // Convert the UV coordinates to pixel coordinates
        int x = static_cast<int>(u * (m_pSurface->w - 1));  // Map U to [0, width-1]
        int y = static_cast<int>(v * (m_pSurface->h - 1));  // Map V to [0, height-1]

        // Ensure pixel coordinates are within bounds (just for safety)
        x = std::clamp(x, 0, m_pSurface->w - 1);
        y = std::clamp(y, 0, m_pSurface->h - 1);

        // Calculate the 1D index from the 2D (x, y) coordinates
        int index = y * m_pSurface->w + x;

        // Extract the pixel from the 1D surface pixel array
        uint32_t pixel = m_pSurfacePixels[index];

        // Extract RGBA components from the pixel
        uint8_t r, g, b, a;
        SDL_GetRGBA(pixel, m_pSurface->format, &r, &g, &b, &a);

        //SDL_Log("Sampling texture at UV: (%f, %f)", u, v);  // Debug UVs

        // Return the color as a ColorRGB object, normalized to the [0, 1] range
        return ColorRGB(r , g , b );
    }


}
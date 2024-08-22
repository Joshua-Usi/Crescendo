#pragma once

#include "common.hpp"
#include "Component.hpp"
#include "cs_std/math/math.hpp"
#include "Rendering/Vulkan/ResourceManager.hpp"

#include <functional>
#include <stack>

CS_NAMESPACE_BEGIN
{
	struct ParticleEmitter : public Component
	{
		struct ParticleShaderRepresentation
		{
			cs_std::math::vec3 position;
			float deathTime;
		};

		struct Particle
		{
			cs_std::math::vec3 position;
			cs_std::math::vec3 velocity;
			float deathTime;
			uint32_t dummy; // can use it later, this just pads it to 32 bytes

			bool IsAlive(float currentTime) const { return currentTime < deathTime; }
			ParticleShaderRepresentation CreateShaderRepresentation() const { return { position, deathTime }; }
		};
		// How particles are updated every frame, arguments are (current time, delta time, current particle), current time is useful for time based effects
		std::function<void(float, float, Particle&)> updateFunction;
		// How particles spawn every time one is made, argument is (current time)
		std::function<Particle(float)> spawnFunction;

		std::vector<Particle> particles;

		// Hard limit for particles
		uint32_t maxParticles, liveParticleCount;
		// In seconds, if less than a frame, will spawn multiple particles
		float emissionRate, accumulator;
		// The total time the particle system has ticked throught
		float runTime;

		Vulkan::TextureHandle texture;

		ParticleEmitter(uint32_t maxParticles, float emissionRate, std::function<void(float, float, Particle&)> updateFunction, std::function<Particle(float)> spawnFunction, Vulkan::TextureHandle texture)
			: runTime(0.0f), maxParticles(maxParticles), liveParticleCount(0), emissionRate(emissionRate), accumulator(0.0f), particles(maxParticles),
			updateFunction(updateFunction), spawnFunction(spawnFunction), texture(texture)
		{}

		void Update(float dt)
		{
			// Update time
			accumulator += dt;
			runTime += dt;

			// Emit particles
			while (accumulator >= emissionRate)
			{
				accumulator -= emissionRate;
				if (liveParticleCount < maxParticles)
				{
					particles[liveParticleCount] = spawnFunction(runTime);
					liveParticleCount++;
				}
			}

			// Update particles
			for (uint32_t i = 0; i < liveParticleCount; i++)
			{
				if (runTime < particles[i].deathTime)
				{
					updateFunction(runTime, dt, particles[i]);
				}
				else
				{
					liveParticleCount--;
					std::swap(particles[i], particles[liveParticleCount]);
				}
			}
		}

	};
}
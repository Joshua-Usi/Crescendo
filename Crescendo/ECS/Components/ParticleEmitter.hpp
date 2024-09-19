#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "cs_std/math/math.hpp"
#include <functional>

CS_NAMESPACE_BEGIN
{
	struct ParticleEmitter : public Component
	{
	public:
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
		// In seconds, if less than a frame, will spawn multiple particles, if 0, will not spawn any particles
		float emissionRate;
		// How many particles are spawned per emission
		uint32_t particlesPerEmission;

		float accumulator;

		bool isActive;

		ParticleEmitter(uint32_t maxParticles, float emissionRate, uint32_t particlesPerEmission, std::function<void(float, float, Particle&)> updateFunction, std::function<Particle(float)> spawnFunction)
			: maxParticles(maxParticles), liveParticleCount(0), emissionRate(emissionRate), particlesPerEmission(particlesPerEmission), accumulator(0.0f),
			updateFunction(updateFunction), spawnFunction(spawnFunction), isActive(true)
		{}
	private:
		void RemoveDeadParticles(float currentTime)
		{
			for (uint32_t i = 0; i < liveParticleCount; i++)
			{
				if (!particles[i].IsAlive(currentTime))
				{
					liveParticleCount--;
					std::swap(particles[i], particles[liveParticleCount]);
				}
			}
		}

		void AttemptParticleSpawn(float currentTime)
		{
			if (!isActive) return;

			while (accumulator >= emissionRate)
			{
				accumulator -= emissionRate;
				for (uint32_t i = 0; liveParticleCount < maxParticles && i < particlesPerEmission; i++, liveParticleCount++)
				{
					if (liveParticleCount >= particles.size())
					{
						particles.push_back(spawnFunction(currentTime));
						// Ensure the particle buffer doesn't get too large, wasting memory
						if (particles.capacity() > maxParticles) particles.resize(maxParticles);
					}
					else
					{
						particles[liveParticleCount] = spawnFunction(currentTime);
					}
				}
			}
		}

		void UpdateParticles(float currentTime, float dt)
		{
			for (uint32_t i = 0; i < liveParticleCount; i++)
			{
				updateFunction(currentTime, dt, particles[i]);
			}
		}
	public:
		void Update(float currentTime, float dt)
		{
			accumulator += dt;
			RemoveDeadParticles(currentTime);
			if (isActive)
				AttemptParticleSpawn(currentTime);
			UpdateParticles(currentTime, dt);
		}
	};
}
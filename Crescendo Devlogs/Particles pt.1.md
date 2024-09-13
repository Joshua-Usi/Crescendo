*The warmth of the campfire, fills you with determination*
![[Particles pt.1 - 2.png]]

Most modern games can handle millions of particles with a GPU-driven particle system, with fancy effects like turbulence. We will not be building a fully fledged particle system like that... at least in this part. We will create a simple system that allows us to get particles on the screen and move it in ways we want. This system isn't too bad, it's single threaded and so far handles 100,000 particles with ease.
## Componentizing particles
Since my engine is ECS, particles will be componentized into a `ParticleEmitter` component
```C++
struct ParticleEmitter : public Component
{
	// How particles are updated every frame, arguments are (current time, delta time, current particle), current time is useful for time based effects
	std::function<void(float, float, Particle&)> updateFunction;
	// How particles spawn every time one is made, argument is (current time)
	std::function<Particle(float)> spawnFunction'
	std::vector<Particle> particles;
	// Hard limit for particles
	uint32_t maxParticles, liveParticleCount;
	// In seconds, if less than a frame, will spawn multiple particles, if 0, will not spawn any particles
	float emissionRate;
	// How many particles are spawned per emission
	uint32_t particlesPerEmission;
	float accumulator;
	bool isActive;
}
```
*192 bytes, pretty nice and round*

A `Particle` is a super simple struct with position, velocity and the time of death of a particle, Just to pad it out nicely, I added a dummy member so it becomes 32 bytes. This could be used for other pieces of data later. Particles also need their shader representation, packed down to minimise memory usage
```C++
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
```
## Updating particles
In the `ParticleEmitter` component. there's two fields of interest, `spawnFunction` and `updateFunction`. As their names imply, spawn function runs every particle emission and returns a particle, while update takes a particle and applies an update to it. Here's an example particle emitter that creates a firework effect
```c++
// 12,000 is the maximum number of particles
// 5.0f seconds per emission
// 10,000 particles are emitted per emission
particleEmitter.EmplaceComponent<ParticleEmitter>(12000, 5.0f, 10000,
	// Updae function
	[](float currentTime, float dt, ParticleEmitter::Particle& p) {
		p.velocity *= 0.95f;
		p.velocity.y += 5.0f * dt;
		p.position += p.velocity * dt;
	},
	// Spawn function
	[](float currentTime) {
		ParticleEmitter::Particle p;

		float radius = math::random(80.0f, 100.0f);

		float u = math::random<float>(0.0f, 1.0f);
		float v = math::random<float>(0.0f, 1.0f);

		float theta = 2.0f * math::pi<float>() * u; // azimuthal angle
		float phi = math::acos(2.0 * v - 1.0); // polar angle

		float x = radius * sinf(phi) * cosf(theta);
		float y = radius * sinf(phi) * sinf(theta);
		float z = radius * cosf(phi);

		p.position = math::vec3(0.0f);
		p.velocity = math::vec3(x, y, z);
		p.deathTime = currentTime + math::random<float>(3.0f, 5.0f);
		return p;
	}
);
```
![[Particles pt.1 - 1.png]]
*Oooh* *Ahhh*

As for updating the particle system internally there's only 4 steps: 
1. Update the accumulator
2. Remove dead particles
3. Attempt particle spawns
4. Update the particles

Funnily enough, the actual implementation looks like that
```C++
void Update(float currentTime, float dt)
{
	accumulator += dt;
	RemoveDeadParticles(currentTime);
	AttemptParticleSpawn(currentTime);
	UpdateParticles(currentTime, dt);
}
```
### Removing dead particles
To efficiently remove dead particles, we swap the last particle with the dead particle and decrement by 1. This is a super fast O(1) operation called swap-n-pop (or at least that's what my friend calls it)
```C++
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
```
### Attempting a particle spawn
Spawning particles should only be attempted if the particle system is active, hence why we have the if statement guarding it. There's a few tricks in here, ranging from that long for statement to the capacity limiter.
```C++
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
```
### Updating particles
And the good ol' updating particles method, is a simple for loop calling the update function
```C++
void UpdateParticles(float currentTime, float dt)
{
	for (uint32_t i = 0; i < liveParticleCount; i++)
	{
		updateFunction(currentTime, dt, particles[i]);
	}
}
```

Finally, our ECS manager loops over each emitter and updates them accordingly
```C++
currentScene.entityManager.ForEach<ParticleEmitter>([&](ParticleEmitter& emitter) {
	emitter.Update(GetTime<float>(), dt);
});
```
## Rendering particles
My favourite part of this was working on the rendering aspect. I chose not to use instancing and decided to generate the vertices on the fly
### Render component
Particles need a render component, so far it's just a simple texture. Later on in a follow-up article I will discuss adding custom particle shaders.
```C++
struct ParticleRenderer : Component
{
	Vulkan::TextureHandle texture;
};
```
### Gathering particles
We need to gather our render data about the particles to send to shaders
```C++
struct ParticleEmitterRenderData
{
	// start index of the particle data for this emitter and the number of particles 
	uint32_t startIdx, count;
	Vulkan::TextureHandle texture;
	uint32_t modelID;
};

std::vector<ParticleEmitter::ParticleShaderRepresentation> particles;
std::vector<ParticleEmitterRenderData> particleEmitters;

currentScene.entityManager.ForEach<Transform, ParticleEmitter, ParticleRenderer>([&](Transform& transform, ParticleEmitter& emitter, ParticleRenderer& renderer) {
	meshTransforms.push_back(transform.GetModelMatrix());
	particleEmitters.emplace_back(particles.size(), emitter.liveParticleCount, renderer.texture, meshTransforms.size() - 1);
	for (const ParticleEmitter::Particle& particle : emitter.particles)
	{
		particles.push_back(particle.CreateShaderRepresentation());
	}
});
```

In the setup stage, I created a particle buffer to store all particles across all emitters, this is what the render data is needed for, to store the offsets of each system. Although 1024 particles seems pitifully low, not mentioned yet is the automatic buffer resizer I have implemented, letting us scale up to as many particles as we desire.
```C++
#define CS_STARTING_PARTICLE_COUNT 1024

for (uint32_t i = 0; i < frameManager.GetFrameCount(); i++)
{
	...
	// Create particle buffer
	particleBufferHandle.push_back(resourceManager.CreateBuffer(sizeof(ParticleEmitter::ParticleShaderRepresentation) * CS_STARTING_PARTICLE_COUNT, VK_SHADER_STAGE_ALL));
}
```

Next up, we record the command buffers for the particle shader.
```C++
// In our main render pass
for (uint32_t i = 0; i < particleEmitters.size(); i++ )
{
	ParticleEmitterRenderData& data = particleEmitters[i];
	const struct ParticleVertexParams {
		uint32_t cameraIdx, transformBufferIdx, particleBufferIdx, particleStartingIdx;
	} particleParams {
		0, transformsHandle[currentFrameIndex].GetIndex(), particleBufferHandle[currentFrameIndex].GetIndex(), data.startIdx
	};
	const struct ParticleFragmentParams {
		uint32_t diffuseTexIdx;
	} particleParamsFrag {
		data.texture.GetIndex()
	};
	cmd.PushConstants(particlePipeline, particleParams, VK_SHADER_STAGE_VERTEX_BIT);
	cmd.PushConstants(particlePipeline, particleParamsFrag, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ParticleVertexParams));
	// One quad for each particle, hence 6 vertices
	cmd.Draw(data.count * 6, 1, 0, data.modelID);
}
```
The last line, where I call `cmd.Draw()`, notice how I render 6 times the number of particles, and only 1 instance. This is the key to the renderer. I also haven't bound any vertex buffers, so where are we getting our vertex positions and texture coordinates?
### The vertex shader
Since we haven't bound any vertex position or texture coordinates, we don't have any vertex location data, but it's possible to generate the square mesh using the vertex IDs. This is the exact same way I render post-processing overlays without having to bind a square mesh. The only downside to this is that the triangles for the mesh are generated with different winding orders, so backface culling must be disabled, which is fine for particles since these will be billboarded.

```C++
uint vertexIndex = gl_VertexIndex % 6;

oTexCoord = vec2(vertexIndex & 1, mod((vertexIndex + 1) / 3, 2));
vec3 position = vec3(oTexCoord - 0.5, 0.0);
```
Next up is billboarding code, it's pretty simple. Take the right and up vectors of the `ModelView` matrix. It's pretty simple but easy to get wrong. don't forget to note your spaces as mentioned in [[Know your space]].
```c++
// Bindless code
mat4 view = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx];
mat4 projection = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx + 1];
mat4 model = GetResource(TransformBuffer, transformBufferIdx).transformData[gl_InstanceIndex];
mat4 modelView = view * model;

vec3 cameraRight_ws = vec3(modelView[0][0], modelView[1][0], modelView[2][0]);
vec3 cameraUp_ws = vec3(modelView[0][1], modelView[1][1], modelView[2][1]);

vec3 position_ws = (cameraRight_ws * scaledPosition.x) + (cameraUp_ws * scaledPosition.y);

gl_Position = proj * modelView * vec4(particle.xyz + position_ws, 1.0);
```
### The fragment shader
The fragment shader is pretty simple, take in a texture and texture coords and render out to the screen
```C++
#version 460
#include "bindless.glsl"

layout (location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	layout(offset = 20) uint diffuseTexIdx;
};

void main() {
	vec4 texelColor = texture(uTextures2D[diffuseTexIdx], iTexCoord);
	oColor = texelColor;
}
```

And blam, you have yourself a basic, CPU-driven particle system. It may not be fancy or have insane numbers of particles but it's a basic implementation nonetheless and will be extended in a later article. For now, here is 160,000 particles running on a single thread at 184fps
![[Particles pt.1 -3.png]]*Probably needs a bit of antialiasing
#version 460
#include "bindless.glsl"

layout (location = 0) out vec2 oTexCoord;
layout (location = 1) out float oParticleDeathTime;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

layout (push_constant) uniform PushConstants {
	uint cameraIdx; // index refers to view and the next index the projection
	uint transformBufferIdx;
	uint particleBufferIdx;
	uint particleStartingIndex; // Offset into the particle buffer
	float currentTime;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformData[]; });
RegisterBuffer(std430, readonly, ParticleBuffer, { vec4 particleData[]; }); // x, y, z, deathTime

void main() {
	uint vertexIndex = gl_VertexIndex % 6;
	uint particleID = gl_VertexIndex / 6;

	mat4 view = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx];
	mat4 projection = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx + 1];
	mat4 model = GetResource(TransformBuffer, transformBufferIdx).transformData[gl_InstanceIndex];
	mat4 modelView = view * model;

	vec3 cameraRight_ws = vec3(modelView[0][0], modelView[1][0], modelView[2][0]);
    vec3 cameraUp_ws = vec3(modelView[0][1], modelView[1][1], modelView[2][1]);
	
	vec4 particle = GetResource(ParticleBuffer, particleBufferIdx).particleData[particleStartingIndex + particleID];
	
	oParticleDeathTime = particle.w;
	oTexCoord = vec2(vertexIndex & 1, mod((vertexIndex + 1) / 3, 2));
	
	vec3 scaledPosition = vec3((oTexCoord - 0.5) * 0.5, 0.0);
	vec3 worldPosition = (cameraRight_ws * scaledPosition.x) + (cameraUp_ws * scaledPosition.y);
	float scale = map(particle.w, currentTime, currentTime + 3.0, 0.0, 1.0);

	gl_Position = projection * view * model * vec4(particle.xyz + worldPosition * scale, 1.0);
}
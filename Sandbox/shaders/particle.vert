#version 460
#include "bindless.glsl"

layout (location = 0) out vec2 oTexCoord;
layout (location = 1) out float oParticleLifetime;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

layout (push_constant) uniform PushConstants {
	// index refers to view and the next index the projection
	// They must be successive
	uint cameraIdx;
	uint transformBufferIdx;
	uint particleBufferIdx;
	// Offset into the particle buffer
	uint particleStartingIndex;
	float currentTime;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformData[]; });
// x, y, z, deathTime
RegisterBuffer(std430, readonly, ParticleBuffer, { vec4 particleData[]; });

void main() {
	uint vertexIndex = gl_VertexIndex % 6;
	uint particleID = particleStartingIndex + gl_VertexIndex / 6;

	vec4 particle = GetResource(ParticleBuffer, particleBufferIdx).particleData[particleID];
	oParticleLifetime = map(particle.w, currentTime, currentTime + 4.0, 0.0, 1.0);

	mat4 m = GetResource(TransformBuffer, transformBufferIdx).transformData[gl_InstanceIndex];
	mat4 v = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx];
	mat4 p = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx + 1];
	mat4 mv = v * m;

	vec3 cameraRight_ws = vec3(mv[0][0], mv[1][0], mv[2][0]);
    vec3 cameraUp_ws = vec3(mv[0][1], mv[1][1], mv[2][1]);
	
	oTexCoord = vec2(vertexIndex & 1, mod((vertexIndex + 1) / 3, 2));
	
	vec2 scaledPosition = oTexCoord - 0.5;
	vec3 positiom_ws = (cameraRight_ws * scaledPosition.x) + (cameraUp_ws * scaledPosition.y);
	float scale = map(oParticleLifetime, 0.0, 1.0, 0.0, 1.0);

	gl_Position = p * mv * vec4(particle.xyz + positiom_ws * scale, 1.0);
}
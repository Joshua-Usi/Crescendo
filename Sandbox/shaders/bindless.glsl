#extension GL_EXT_nonuniform_qualifier : enable

#define BINDLESS 1

#define BINDLESS_DESCRIPTOR_SET 0

#define BINDLESS_UNIFORM_BINDING 0
#define BINDLESS_STORAGE_BINDING 1
#define BINDLESS_SAMPLER_BINDING 2

#define GetLayoutVariableName(Name) u##Name##Register

#define RegisterUniform(Name, Struct) \
	layout(set = BINDLESS_DESCRIPTOR_SET, binding = BINDLESS_UNIFORM_BINDING) uniform Name Struct GetLayoutVariableName(Name)[]

#define RegisterBuffer(Layout, BufferAccess, Name, Struct) \
	layout(Layout, set = BINDLESS_DESCRIPTOR_SET, binding = BINDLESS_STORAGE_BINDING) BufferAccess buffer Name Struct GetLayoutVariableName(Name)[]

#define GetResource(Name, Index) GetLayoutVariableName(Name)[Index]

// Register empty resources
// to be compliant with the pipeline layout
// even if the shader does not use all the descriptors
RegisterUniform(DummyUniform, { uint ignore; });
RegisterBuffer(std430, readonly, DummyBuffer, { uint ignore; });

layout(set = BINDLESS_DESCRIPTOR_SET, binding = BINDLESS_SAMPLER_BINDING) uniform sampler2D uTextures2D[];
layout(set = BINDLESS_DESCRIPTOR_SET, binding = BINDLESS_SAMPLER_BINDING) uniform samplerCube uTexturesCube[];
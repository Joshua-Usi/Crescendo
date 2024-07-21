## Enable Descriptor Indexing
- Self explained
## Create a bindless descriptor set layout
Create the layout
```C++
constexpr std::array<VkDescriptorType, 3> types{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
// For simplicity, we treat buffers and uniforms as the same thing, so the true maximum is 2x the maxBuffers
const std::array<uint32_t, 3> setsSizes = { { spec.maxBuffers, spec.maxBuffers, spec.maxImages } };
std::array<VkDescriptorSetLayoutBinding, 3> bindings = {};	
std::array<VkDescriptorBindingFlags, 3> flags = {};
std::array<VkDescriptorPoolSize, 3> sizes = {};
for (uint8_t i = 0; i < types.size(); i++)
{
	bindings[i] = Create::DescriptorSetLayoutBinding(i, types[i], setsSizes[i], VK_SHADER_STAGE_ALL);
	flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	sizes[i] = { types[i], setsSizes[i] };
}
const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = Create::DescriptorSetLayoutBindingFlagsCreateInfo(flags);
const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = Create::DescriptorSetLayoutCreateInfo(&bindingFlags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, bindings);
vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &layout);
```
Create the pool
```C++
// Create the pool
const VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, 1, sizes);
vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
```
Create the global set
```C++
// Create the global set
const VkDescriptorSetAllocateInfo allocateInfo = Create::DescriptorSetAllocateInfo(pool, &layout);
vkAllocateDescriptorSets(device, &allocateInfo, &set);
```
## Manage bindless descriptors and communication
- Use handles / indices to manage where data is stored
- Whenever we want to add a new resource to a descriptor set, we need to write it
	- At this point, no buffers are even associated with the descriptor set, we are just letting vulkan know that it is valid
- When we write a texture, it just writes directly to a texture array, we can access each texture through indices
- When we write a buffer, we are really writing to an array of buffers. Hence that is why when we go to access buffers in a shader, we need an index here
	- GetResource(TransformBuffer, index) because it tells the shader which buffer to use
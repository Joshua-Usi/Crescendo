// #define USE_DIFFUSE_MAP : use the diffuse texture, otherwise uses a solid color
// #define USE_METALLIC_MAP : use the metallic texture, otherwise uses a metallic value
// #define USE_ROUGHNESS_MAP : use the roughness map, otherwise use a roughness value
// #define USE_NORMAL_MAP : use the normal texture, otherwise the default normal. does not need to be defined if USE_LIGHTING is not
// #define USE_AO_MAP : use the ambient occlusion map, otherwise uses a ao value. usually 1.0f
// #define USE_EMISSIVE_MAP : uses the emissive texture, otherwise uses an emissive color

#if defined(USE_DIFFUSE_MAP) || defined(USE_NORMAL_MAP) || defined(USE_METALLIC_MAP) || defined(USE_ROUGHNESS_MAP) || defined(USE_AO_MAP) || defined(USE_EMISSIVE_MAP)
    #define USE_TEXTURE_MAPS 1
#else
    #define USE_TEXTURE_MAPS 0
#endif
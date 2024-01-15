 *Man's have dreamed of strongly-typed coordinate spaces but alas, it's such a pain and makes code ugly so generally not worth I think.* - omnisci3nt

Often times, you can discover infuriating shader bugs due to mixing the different coordinate spaces, here are a few and how to distinguish them
## Model Space
This coordinate system is where the model originates before any transforms. Hence it is equivalent to just taking the raw vertex positions.
## World Space
This space denotes where an object is placed around a scene. It is equivalent to:
- **Model Space** * model matrix
## View Space (Camera Space)
View Space or Camera Space denotes the coordinate system from the cameras point of view. It is equivalent to:
- **World Space** * view matrix
## Clip Space
Clip space is an intermediate space within the render pipelines that determines if an object is out of view and should not be rendered. It is derived from:
- **View Space** * projection matrix
## NDC
NDC or Normalized Device Coordinates denotes how the rendered image is scaled to the screen or viewport. It adjusts for aspect ratio and sets up pixels for the final displaying render. In NDC the xyz components are normalized to between -1 and 1
## Screen Space
Screen space involves scaling and translating the coordinates to map onto the screen. This is done in Vulkan automatically by pipelines
## Tangent Space
Tangent Space defines how normals and normal maps are transformed. Tangent space involves a normal, tangent and bitangent (which can be derived from normal and tangent)
## How do I stop mixing these spaces???
One approach I read about was to append the variable names with the space name

Hence:
Model Space = `_ms`
World Space = `_ws`
View Space = `_vs`
Screen Space = `_ss`
Tangent Space = `_ts`
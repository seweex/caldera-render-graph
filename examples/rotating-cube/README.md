# Caldera: Rotating Cube

This is a completed Vulkan application, that show a Render Graph usage.
Here are the Render Graphs and its passes used in this example.

Output Example:

<img src="../../assets/cube.gif" width=200>

## Load Graph
It fills the geometry buffers. It's executed only at the first frame.

Managed resources:
- `Staging Buffer` needs as proxy between the CPU and GPU memories
- `Vertex Buffer` stores vertices of the cube
- `Index Buffer` contains _uint32_t_ indices for the cube vertices  

Passes:
- **Staging Pass**: maps the staging buffer and writes vertices and indices to it
- **Transfer Pass**: uses Vulkan transfer operations to copy data from the staging buffer to 
the target vertex and index buffers


## Draw Graph

This one draws the cube and presents it. This graph is executed every frame. 
On the first, scheduler waits for timeline semaphore signaled after load operations.

Managed resources:
- `Vertex Buffer` is read as a vertex input
- `Index Buffer` is used for indexing the vertices
- `Swapchain Image` for drawing the cube

Passes:
- **Draw Pass**: uses the vertex and index buffers as an input data and draws it
- **Present Pass**: makes the _ColorAttachment_ → _PresentSource_ transition for the
current swapchain image

## Resources

You can edit the [geometry](../common/include/caldera-examples-common/mesh.h) or 
the [shaders](shaders), load textures and other. 
Pipeline accepts input data as:
- `binding 0, location 0`: vec3, position
- `binding 0, location 1`: vec2, uv
- `binding 0, location 2`: vec3, normals

And push constants:
- `mat4` - camera view matrix

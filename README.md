# Engine1

![alt text](https://warsztat.gd/screens/147d87ff0736e81737c7bff66a782c21.png)

The aim of this project is to create a renderer which almost matches Path Tracing in quality, while generating noiseless images hundreds times faster. 
Instead of tracing thousands of rays per pixel to get soft shadows and diffuse reflections, it uses "inteligent" blur in screen space. 
It also uses standard Deferred Rendering for rasterizing main image. One of the project's fundamental rules is that everything is dynamic - nothing is baked.

<b>Working features:</b><br>
&bull; Deferred Rendering <br>
&bull; Light reflections/refractions <br>
&emsp;&emsp;&middot; Fully dynamic <br>
&emsp;&emsp;&middot; Diffuse - for rough surfaces <br>
&emsp;&emsp;&middot; Multiple refractions/reflections <br>
&emsp;&emsp;&middot; Blur level depanding on surface to object distance <br>
&bull; Shadows <br>
&emsp;&emsp;&middot; Fully dynamic <br>
&emsp;&emsp;&middot; Soft - depanding on light source radius, distance to shadow caster <br>
&emsp;&emsp;&middot; "Unfinite " precision - tiny/huge objects cast proper soft shadows, light source can be 1000 km away<br>
&emsp;&emsp;&middot; Visible in reflections/refractions <br>
&emsp;&emsp;&middot; Correct shadows from transparent objects - depanding on alpha texture <br>
&bull; Physically Based Rendering - only one material, which supports all effects <br>
&bull; Loading/saving scenes, meshes, generating BVH trees <br>
&bull; Tonemapping, bloom <br>

<b>Future plans: </b><br>
&bull; Switching from DX11 do DX12 - to allow for indexing meshes inside shaders, support for huge scenes <br>
&bull; Dynamic Global Illumination <br>
&bull; Support for skeletal animation <br>
&bull; Support for arbitrary animation - ex. based on physics - cloths, fluids <br>
&bull; Dynamic caustics <br>
&bull; Volumetric effects, patricles - smoke, fire, clouds <br>
&bull; Creating a sandbox game for testing <br>

Note: FPS for above image is around 23.

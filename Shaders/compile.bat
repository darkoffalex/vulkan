C:\VulkanSDK\1.2.148.1\Bin\glslangValidator.exe -V base-phong.vert -o base-phong.vert.spv
C:\VulkanSDK\1.2.148.1\Bin\glslangValidator.exe -V base-phong.geom -o base-phong.geom.spv
C:\VulkanSDK\1.2.148.1\Bin\glslangValidator.exe -V base-phong.frag -o base-phong.frag.spv

C:\VulkanSDK\1.2.148.1\Bin\glslangValidator.exe  -V raytrace.rgen -o raytrace.rgen.spv --target-env vulkan1.2
C:\VulkanSDK\1.2.148.1\Bin\glslangValidator.exe -V raytrace.rmiss -o raytrace.rmiss.spv --target-env vulkan1.2
C:\VulkanSDK\1.2.148.1\Bin\glslangValidator.exe -V raytrace.rchit -o raytrace.rchit.spv --target-env vulkan1.2

pause
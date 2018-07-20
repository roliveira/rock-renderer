
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unordered_map>

#include "ospray/ospray.h"
#include "ospray/ospcommon/vec.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "io/reader.hpp"


using namespace ospcommon;



void setup_volume(OSPVolume volume, std::vector<unsigned char> volume_data, OSPTransferFunction transfer_fcn, const vec3i &dims) 
{
	const std::vector<vec3f> colors = {
		vec3f(0.0, 0.0, 0.0),
		vec3f(0.9, 0.9, 0.9)
	};

	const std::vector<float> opacities = {0.01f, 0.9f};
	OSPData colors_data = ospNewData(colors.size(), OSP_FLOAT3, colors.data());
	ospCommit(colors_data);
	OSPData opacity_data = ospNewData(opacities.size(), OSP_FLOAT, opacities.data());
	ospCommit(opacity_data);

	const vec2f value_range(0.0, 1.0);
	ospSetData(transfer_fcn, "colors", colors_data);
	ospSetData(transfer_fcn, "opacities", opacity_data);
	ospSetVec2f(transfer_fcn, "valueRange", (osp::vec2f&)value_range);
	ospCommit(transfer_fcn);

	ospSetString(volume, "voxelType", "uchar");
	ospSetVec3i(volume, "dimensions", (osp::vec3i&)dims);
	ospSetObject(volume, "transferFunction", transfer_fcn);

	ospSetRegion(volume, volume_data.data(), osp::vec3i{0, 0, 0}, (osp::vec3i&)dims);
	ospCommit(volume);
}



int main(int argc, const char **argv) 
{
    if (argc < 3)
    {
        std::cerr <<  "Usage: " << argv[0] << " INFILE OUTFILE" << std::endl;
        return EXIT_FAILURE;
    }

    ospInit(&argc, argv);

    std::string ifname = argv[1];
    std::string ofname = argv[2];

    std::unordered_map<std::string, float> header = ReadASCIIHeader(ifname.c_str());
    std::vector<unsigned char> volume_data = ReadASCII(ifname.c_str(), header["nx"]*header["ny"]*header["nz"]);

    const vec3i volume_dims(header["nx"], header["ny"], header["nz"]);
	OSPTransferFunction transfer_fcn = ospNewTransferFunction("piecewise_linear");

	OSPVolume volume = ospNewVolume("block_bricked_volume");
	ospSetObject(volume, "transferFunction", transfer_fcn);

	const vec2i img_size(4096, 4096);
	const vec3f cam_pos(-volume_dims[0], -volume_dims[1], 1.25*volume_dims[2]);
	const vec3f cam_dir = vec3f(volume_dims) / 2.f - cam_pos;
	const vec3f cam_up(0, 0, -1);

	// Setup the camera to render from
	OSPCamera camera = ospNewCamera("perspective");
	ospSet1f(camera, "aspect", img_size.x / static_cast<float>(img_size.y));
	ospSetVec3f(camera, "pos", (osp::vec3f&)cam_pos);
	ospSetVec3f(camera, "dir", (osp::vec3f&)cam_dir);
	ospSetVec3f(camera, "up", (osp::vec3f&)cam_up);
	ospCommit(camera);

	setup_volume(volume, volume_data, transfer_fcn, volume_dims);

    // Create the renderer we'll use to render the image
	OSPRenderer renderer = ospNewRenderer("scivis");

	OSPModel model = ospNewModel();
	ospAddVolume(model, volume);
	ospCommit(model);

    // Create an ambient light, which will be used to compute ambient occlusion
	std::vector<OSPLight> lights_list;

	OSPLight ambient_light = ospNewLight(renderer, "ambient");
	ospSet1f(ambient_light, "intensity", 0.2);
	ospCommit(ambient_light);
	lights_list.push_back(ambient_light);

    OSPLight sun_light = ospNewLight(renderer, "distant");
	ospSetVec3f(sun_light, "direction", osp::vec3f{1.f, 1.f, 0.5f});
	ospSetVec3f(sun_light, "color", osp::vec3f{1.f, 1.f, 0.8f});
	ospSet1f(sun_light, "intensity", 0.5);
	ospSet1f(sun_light, "angularDiameter", 1);
	ospCommit(sun_light);
	lights_list.push_back(sun_light);

	OSPLight fill_light = ospNewLight(renderer, "distant");
	ospSetVec3f(fill_light, "direction", osp::vec3f{0.5f, 1.f, 1.5f});
	ospSetVec3f(fill_light, "color", osp::vec3f{1.f, 1.f, 0.8f});
	ospSet1f(fill_light, "intensity", 0.2);
	ospSet1f(fill_light, "angularDiameter", 8);
	ospCommit(fill_light);
	// lights_list.push_back(fill_light);

	OSPData lights = ospNewData(lights_list.size(), OSP_LIGHT, lights_list.data(), 0);
	ospCommit(lights);

    // Setup other renderer params
	ospSetObject(renderer, "model", model);
	ospSetObject(renderer, "camera", camera);
	ospSetObject(renderer, "lights", lights);
	ospSet1i(renderer, "shadowsEnabled", 1);
	ospSet1i(renderer, "spp", 32);
	ospSet1i(renderer, "aoSamples", 16);
	ospSet1i(renderer, "aoTransparencyEnabled", 1);
	ospSetVec4f(renderer, "bgColor", osp::vec4f{0.0f, 0.0f, 0.0f, 0.0f});
	ospCommit(renderer);

	OSPFrameBuffer framebuffer = ospNewFrameBuffer((osp::vec2i&)img_size, OSP_FB_SRGBA, OSP_FB_COLOR);
	ospFrameBufferClear(framebuffer, OSP_FB_COLOR);

	// Finally, render the image and save it out
	ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR);

	const uint32_t *fb = static_cast<const uint32_t*>(ospMapFrameBuffer(framebuffer, OSP_FB_COLOR));
	stbi_write_png(ofname.c_str(), img_size.x, img_size.y, 4, fb, img_size.x * sizeof(uint32_t));
	ospUnmapFrameBuffer(fb, framebuffer);

    return EXIT_SUCCESS;
}

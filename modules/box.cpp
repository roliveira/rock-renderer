//

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <functional>
#include <unordered_map>
#include <random>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"

#include "io/reader.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace rkcommon::math;


long int sub2ind(vec3l ind, vec3l n)
{
    return ind.z*n.x*n.y - ind.y*n.x + ind.x;
}

vec3l ind2sub(long int ind, vec3l n) 
{
    long int iz = static_cast<long int>(ind/(n.x*n.y));
    long int iy = static_cast<long int>((ind-iz*n.x*n.y)/n.x);
    long int ix = ind - iz*n.x*n.y - iy*n.x;

    vec3l sub = {ix, iy, iz};

    return sub;
}


int main(int argc, const char **argv)
{
    if (argc < 6)
    {
        std::cerr <<  "Usage: " << argv[0] << " INFILE OUTFILE NX NY NZ" << std::endl;
        return EXIT_FAILURE;
    }

    std::string ifname = argv[1];
    std::string ofname = argv[2];

    long int nx = std::atoi(argv[3]);
    long int ny = std::atoi(argv[4]);
    long int nz = std::atoi(argv[5]);
    long int n  = nx*ny*nz;

    // std::vector<unsigned char> volume_data = ReadASCII(ifname.c_str(), nx*ny*nz);
    std::vector<int> volume_data = ReadRAW(ifname.c_str(), n);

    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR) return init_error;

    {
        // create and setup the geometry and model

        vec3l volume_dims(nx, ny, nz);

        // create the vector of boxes

        std::vector<box3f> boxes;
        std::vector<vec4f> color;

        for (long int i=0; i<nx*ny*nz; ++i) 
        {
            vec3l ijk = ind2sub(i, volume_dims);

            if (255-volume_data[i] > 0)
            {
                vec3l lower = ijk;
                vec3l upper = lower + 1.0;

                vec3f box_color = {0.5f, 0.5f, 0.5f};
                
                boxes.emplace_back(lower, upper);
                color.emplace_back(box_color.x, box_color.y, box_color.z, 1.0f);
            }
        }

        ospray::cpp::Geometry boxGeometry("box");
        boxGeometry.setParam("box", ospray::cpp::CopiedData(boxes));
        boxGeometry.commit();

        ospray::cpp::GeometricModel model(boxGeometry);
        model.setParam("color", ospray::cpp::CopiedData(color));
        model.commit();

        // put the model into a group (collection of models)
        ospray::cpp::Group group;
        group.setParam("geometry", ospray::cpp::CopiedData(model));
        group.commit();

        // put the group into an instance (give the group a world transform)
        ospray::cpp::Instance instance(group);
        instance.commit();

        // put the instance in the world
        ospray::cpp::World world;
        world.setParam("instance", ospray::cpp::CopiedData(instance));

        // create and setup the camera

        const vec2i img_size(4096, 4096);
        const vec3f cam_pos(-volume_dims[0], -volume_dims[1], 1.25*volume_dims[2]);
        const vec3f cam_dir = vec3f(volume_dims) / 2.f - cam_pos;
        const vec3f cam_up(0, 0, -1);
        
        ospray::cpp::Camera camera("perspective");
        camera.setParam("aspect",    img_size.x / static_cast<float>(img_size.y));
        camera.setParam("position",  cam_pos);
        camera.setParam("direction", cam_dir);
        camera.setParam("up",        cam_up);
        camera.commit();

        // create and setup light for Ambient Occlusion
        ospray::cpp::Light ambient("ambient");
        ambient.setParam("intensity", 0.35f);
        ambient.setParam("visible", false);
        ambient.commit();
        
        ospray::cpp::Light distant("distant");
        distant.setParam("color", vec3f{1.0f, 1.0f, 1.0f});
        distant.setParam("intensity", 3.14f);
        distant.setParam("direction", vec3f(0.8f, 0.6f, 0.3f));
        distant.commit();
        
        std::vector<ospray::cpp::Light> lights{distant, ambient};
        world.setParam("light", ospray::cpp::CopiedData(lights));
        world.commit();

        // create renderer, choose Scientific Visualization renderer
        ospray::cpp::Renderer renderer("scivis");

        // complete setup of renderer
        renderer.setParam("pixelSamples", 10);
        renderer.setParam("shadows", true);
        renderer.setParam("aoSamples", 16);
        renderer.setParam("backgroundColor", 1.0f); // white, transparent
        renderer.commit();

        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(img_size.x, img_size.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        // render one frame
        framebuffer.renderFrame(renderer, camera, world);

        uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        stbi_write_png(ofname.c_str(), img_size.x, img_size.y, 4, fb, img_size.x * sizeof(uint32_t));
        framebuffer.unmap(fb);
    }

    ospShutdown();

    return EXIT_SUCCESS;
}

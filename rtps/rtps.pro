HEADERS += \
    ../test/sphparametergroup.h \
    ../test/particleeffectparametergroup.h \
    ../test/ParamParser.h \
    ../test/mainwindow.h \
    ../test/glwidget.h \
    ../test/floatslider.h \
    ../test/aiwrapper.h \
    ../rtpslib/render/SSEffect.h \
    ../rtpslib/render/ShaderLibrary.h \
    ../rtpslib/render/Shader.h \
    ../rtpslib/render/ParticleEffect.h \
    ../rtpslib/render/MeshEffect.h \
    ../rtpslib/render/StreamlineEffect.h \
    ../rtpslib/render/Camera.h \
    ../rtpslib/render/Quaternion.h \
    ../rtpslib/render/RenderUtils.h

SOURCES += \
    ../test/sphparametergroup.cpp \
    ../test/particleeffectparametergroup.cpp \
    ../test/ParamParser.cpp \
    ../test/mainwindow.cpp \
    ../test/mainqt.cpp \
    ../test/glwidget.cpp \
    ../test/floatslider.cpp \
    ../test/aiwrapper.cpp \
    ../rtpslib/render/SSEffect.cpp \
    ../rtpslib/render/Shader.cpp \
    ../rtpslib/render/ParticleEffect.cpp \
    ../rtpslib/render/MeshEffect.cpp \
    ../rtpslib/render/StreamlineEffect.cpp \
    ../rtpslib/render/Camera.cpp \
    ../rtpslib/render/Quaternion.cpp \
    ../rtpslib/render/RenderUtils.cpp

FORMS += \
    ../test/mainwindow.ui

OTHER_FILES += \
    ../rtpslib/render/shaders/vector_vert.glsl \
    ../rtpslib/render/shaders/vector_geom.glsl \
    ../rtpslib/render/shaders/vector_frag.glsl \
    ../rtpslib/render/shaders/sprite_vert.glsl \
    ../rtpslib/render/shaders/sprite_tex_frag.glsl \
    ../rtpslib/render/shaders/sprite_smoke_frag.glsl \
    ../rtpslib/render/shaders/sphere_vert.glsl \
    ../rtpslib/render/shaders/sphere_tex_frag.glsl \
    ../rtpslib/render/shaders/sphere_light.glsl \
    ../rtpslib/render/shaders/sphere_frag.glsl \
    ../rtpslib/render/shaders/render_water_IBL_vert.glsl \
    ../rtpslib/render/shaders/render_water_IBL_frag.glsl \
    ../rtpslib/render/shaders/render_lit_vert.glsl \
    ../rtpslib/render/shaders/render_lit_frag.glsl \
    ../rtpslib/render/shaders/render_instanced_vert.glsl \
    ../rtpslib/render/shaders/render_instanced_frag.glsl \
    ../rtpslib/render/shaders/normal_vert.glsl \
    ../rtpslib/render/shaders/normal_tex_frag.glsl \
    ../rtpslib/render/shaders/normal_frag.glsl \
    ../rtpslib/render/shaders/mpvertex.glsl \
    ../rtpslib/render/shaders/mpgeometry.glsl \
    ../rtpslib/render/shaders/mpfragment.glsl \
    ../rtpslib/render/shaders/gaussian_blur_y_frag.glsl \
    ../rtpslib/render/shaders/gaussian_blur_x_frag.glsl \
    ../rtpslib/render/shaders/gaussian_blur_vert.glsl \
    ../rtpslib/render/shaders/depth_vert.glsl \
    ../rtpslib/render/shaders/depth_frag.glsl \
    ../rtpslib/render/shaders/curvature_flow.cl \
    ../rtpslib/render/shaders/copy_vert.glsl \
    ../rtpslib/render/shaders/copy_frag.glsl \
    ../rtpslib/render/shaders/boid_tex_frag.glsl \
    ../rtpslib/render/shaders/post_process.vert \
    ../rtpslib/render/shaders/sphere.vert \
    ../rtpslib/render/shaders/render_water_IBL.vert \
    ../rtpslib/render/shaders/render_water_IBL.frag \
    ../rtpslib/render/shaders/render_lit.vert \
    ../rtpslib/render/shaders/render_lit.frag \
    ../rtpslib/render/shaders/render_instanced.vert \
    ../rtpslib/render/shaders/render_instanced.frag \
    ../rtpslib/render/shaders/gaussian_blur_y.frag \
    ../rtpslib/render/shaders/gaussian_blur_x.frag \
    ../rtpslib/render/shaders/gaussian_blur.frag \
    ../rtpslib/render/shaders/draw_vector.vert \
    ../rtpslib/render/shaders/draw_vector.geom \
    ../rtpslib/render/shaders/draw_vector.frag \
    ../rtpslib/render/shaders/curvature_flow.frag \
    ../rtpslib/render/shaders/copy.vert \
    ../rtpslib/render/shaders/copy.frag \
    ../rtpslib/render/shaders/bilateral_blur.frag \
    ../rtpslib/render/shaders/sphere.frag \
    ../rtpslib/render/shaders/normal.vert \
    ../rtpslib/render/shaders/normal.frag \
    ../rtpslib/render/shaders/passthrough.vert \
    ../rtpslib/render/shaders/passthrough.frag

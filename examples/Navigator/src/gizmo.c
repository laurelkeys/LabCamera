//------------------------------------------------------------------------------
//  gizmo.c
//------------------------------------------------------------------------------

#include "gizmo.h"

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "gizmo_glsl.h"

static struct {
    float rx, ry;
    sg_pipeline pip;
    sg_bindings bind;
    sg_buffer vbuf;
    sg_buffer ibuf;
} state;

void init_gizmo() 
{
    state.vbuf = sg_make_buffer(&(sg_buffer_desc){
        .size = 65536,
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .content = NULL,
        .label = "gizmo-vertices"
    });

    /* create an index buffer for the gizmo */
    state.ibuf = sg_make_buffer(&(sg_buffer_desc){
        .size = 32768,
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_STREAM,
        .content = NULL,
        .label = "gizmo-indices"
    });

    /* create shader */
    sg_shader shd = sg_make_shader(gizmo_shader_desc());

    /* create pipeline object */
    state.pip = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers = {
                [0] = {.stride = sizeof(float) * 10, .step_rate = 1, .step_func = SG_VERTEXSTEP_PER_VERTEX }
            },
            .attrs = {
                [ATTR_vs_position] = {.offset = 0, .format = SG_VERTEXFORMAT_FLOAT3, .buffer_index = 0 },
                [ATTR_vs_normal0] = {.offset = 3 * sizeof(float), .format = SG_VERTEXFORMAT_FLOAT3, .buffer_index = 0 },
                [ATTR_vs_color0] = {.offset = 6 * sizeof(float), .format = SG_VERTEXFORMAT_FLOAT4, .buffer_index = 0 }
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT32,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .rasterizer.cull_mode = SG_CULLMODE_BACK,
        .label = "gizmo-pipeline"
    });

    /* setup resource bindings */
    state.bind.vertex_buffers[0] = state.vbuf;
    state.bind.index_buffer = state.ibuf;
}





void draw_gizmo(const float* v_t, const float* mvp, int triangle_count, uint32_t* indices, float* vertices)
{
    static bool init = true;
    if (init)
    {
        init_gizmo();
        init = false;
    }

    sg_update_buffer(state.vbuf, vertices, triangle_count * 10 * sizeof(float));
    sg_update_buffer(state.ibuf, indices, triangle_count * 3 * sizeof(uint32_t));

    /* NOTE: the vs_params_t struct has been code-generated by the shader-code-gen */
    vs_params_t vs_params;
    vs_params.mvp = *((hmm_mat4*)mvp);
    //vs_params.v_t = *((hmm_mat4*)v_t);

    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    //sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, &fs_params, sizeof(fs_params));
    sg_draw(0, triangle_count * 3, 1);
}
#version 130
// Make sure to keep this in sync with sdl/shader.hpp and default.frag!
#define EFFECT_FLIP 1
#define EFFECT_FLOP 2
#define EFFECT_GRAYSCALE 4

// Required by SDL_gpu
attribute vec4 vert_draw_color;
attribute vec3 vert_vertex;
attribute vec2 vert_texture_pos;
attribute vec4 vert_color_mod;
attribute float vert_submerge;
attribute int vert_effects;

uniform mat4 model_view_proj;

varying vec4 frag_color_mod;
varying vec2 frag_texture_pos;
varying float frag_submerge;
varying vec4 frag_draw_color;
flat out int frag_effects;

void main()
{
	frag_texture_pos = vert_texture_pos;
	if ((vert_effects & EFFECT_FLIP) != 0) {
		frag_texture_pos.x = 1 - vert_texture_pos.x;

	}
	if ((vert_effects & EFFECT_FLOP) != 0) {
		frag_texture_pos.y = 1 - vert_texture_pos.y;
	}
	frag_color_mod = vert_color_mod;
	frag_draw_color = vert_draw_color;
	frag_submerge = vert_submerge;
	frag_effects = vert_effects;
	gl_Position = model_view_proj * vec4(vert_vertex, 1.0);
}

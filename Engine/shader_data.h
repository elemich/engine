#ifndef __SHADER_DATA_HEADER__
#define __SHADER_DATA_HEADER__

static const char* unlit_color_vert =
	"attribute vec4 position;\n"
	"varying vec3 vcolor;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"uniform vec3 color;\n"
	"void main(void) {\n"
	"	 vcolor=color;"
	"    gl_Position = projection * modelview * vec4(position.xyz,1.0);\n"
	"}\n";

static const char* unlit_color_frag =
	"varying vec3 vcolor;\n"
	"void main(void) {\n"
	"    gl_FragColor = vec4(vcolor,1.0);\n"
	"}\n";


static const char* unlit_vert =
	"attribute vec4 position;\n"
	"attribute vec3 color;\n"
	"varying vec3 vcolor;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"void main(void) {\n"
	"	 vcolor=color;"
	"    gl_Position = projection * modelview * vec4(position.xyz,1.0);\n"
	"}\n";

static const char* unlit_frag =
	"varying vec3 vcolor;\n"
	"void main(void) {\n"
	"    gl_FragColor = vec4(vcolor,1.0);\n"
	"}\n";

/////////////////////////////////////////////////////////////////////

static const char* unlit_texture_vs =
	"attribute vec4 position;\n"
	"attribute vec2 texcoord;\n"
	"attribute vec3 normal;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"varying vec2 varying_texcoord;\n"
	"varying vec3 varying_normal;\n"
	"\n"
	"void main(void) {\n"
	"    varying_texcoord = texcoord;\n"
	"    varying_normal = normal;\n"
	"    gl_Position = projection * modelview * position;\n"
	"}\n";

static const char* unlit_texture_fs =
	"uniform sampler2D texture;\n"
	"varying vec2 varying_texcoord;\n"
	"varying vec3 varying_normal;\n"
	"\n"
	"void main(void) {\n"
	"    gl_FragColor = texture2D(texture, varying_texcoord);\n"
	"}\n";

//////////////////////////////////////////////////////////////////////////////

static const char* texture_vertex_shaded_vert =
	"attribute vec3 position;\n"
	"attribute vec2 texcoord;\n"
	"attribute vec3 normal;\n"
	"uniform sampler2D texture;\n"
	"uniform bool textured;\n"
	"uniform mat4 modelview;\n"
	"uniform mat4 projection;\n"
	"uniform vec3 lightpos;\n"
	"uniform vec3 lightdiff;\n"
	"uniform vec3 lightamb;\n"
	"varying vec4 varying_color;\n"
	"varying vec4 varying_texcolor;\n"
	"vec3 lightdir;\n"
	"float cosine;\n"
	"float lambert;\n"
	"float luminosity;\n"
	"\n"
	"void main(void) {\n"
	"    vec4 pos = vec4(position,1.0);\n"
	"    varying_texcolor = textured ? texture2D(texture, texcoord) : vec4(1.0,1.0,1.0,1.0);\n"
	"    lightdir = lightpos - vec3(pos);\n"
	"	 cosine = dot(normal, normalize(lightdir));\n"
	"	 lambert = max(cosine, 0.0);\n"
	"    luminosity = 1.0 / (sqrt(length(lightdir)) * sqrt(length(lightdir)));\n"
	"    varying_color = vec4(lightdiff * lightamb * lambert * luminosity,1.0);\n"
	"    gl_Position = projection * modelview * pos;\n"
	"}\n";


static const char* texture_vertex_shaded_frag =
	"varying vec4 varying_texcolor;\n"
	"varying vec4 varying_color;\n"
	"\n"
	"void main(void) {\n"
	"    gl_FragColor = varying_color * varying_texcolor;\n"
	"}\n";




static const char* font_pixsh =
	"attribute vec4 position;\n"
	"attribute vec2 texcoord;\n"
	"varying vec2 v_texcoord;\n"
	"void main(void) {\n"
	"gl_Position = position;\n"
	"v_texcoord = texcoord;\n"
	"}\n";

static const char* font_frgsh =
	"varying vec2 v_texcoord;\n"
	"uniform sampler2D texture;\n"
	"uniform vec4 color;\n"
	"\n"
	"void main(void) {\n"
	"	gl_FragColor = texture2D(texture, v_texcoord);\n"
	"}\n";


#endif //__SHADER_DATA_HEADER__
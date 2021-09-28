#include "Mode.hpp"
#include "ColorTextureProgram.hpp"

#include "GL.hpp"
#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode(char* file);
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	bool pressed[9] = { false };

	int score = 0;
	uint32_t perfect = 0;
	uint32_t good = 0;
	uint32_t bad = 0;

	std::deque<float> beats;
	std::deque<float> active_beats;
	std::deque<int> keymap; //maps keys to active beats. active_beat[i] is on key keymap[i]
	std::vector<int> keys = {0, 1, 2, 3, 4, 5, 6, 7, 8}; //available keys to pick from
	glm::u8vec4 keycolors[9] = { glm::u8vec4(255, 255, 255, 0), glm::u8vec4(255, 255, 255, 0), glm::u8vec4(255, 255, 255, 0), 
								glm::u8vec4(255, 255, 255, 0), glm::u8vec4(255, 255, 255, 0), glm::u8vec4(255, 255, 255, 0), 
								glm::u8vec4(255, 255, 255, 0), glm::u8vec4(255, 255, 255, 0), glm::u8vec4(255, 255, 255, 0) };
	float timer = 0.0f;

	glm::vec2 playable_area_dim = glm::vec2(10.0f, 10.0f);
	float offset = 0.5f;
	glm::vec2 button_dim = glm::vec2(3.0f, 3.0f);

	//music playing:
	char* filename;
	std::shared_ptr< Sound::PlayingSample > music;
	
	//----- opengl assets / helpers ------

	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const& Position_, glm::u8vec4 const& Color_, glm::vec2 const& TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4 * 3 + 1 * 4 + 4 * 2, "PlayMode::Vertex should be packed");

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

};

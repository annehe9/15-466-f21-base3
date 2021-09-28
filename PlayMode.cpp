#include "PlayMode.hpp"

#include "DrawLines.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <deque>
#include <vector>
#include <iterator>
#include <time.h>

//Load< Sound::Sample > loadmusic(LoadTagDefault, []() -> Sound::Sample const * {
	//return 
//});

PlayMode::PlayMode(char* file) {
	filename = file;

	//get beat info
	beats = Sound::parse_beats(*(new Sound::Sample(data_path(filename))));

	//taken from game0
	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of PongMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 4 * 3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 4 * 3 + 4 * 1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1, 1);
		std::vector< glm::u8vec4 > data(size.x * size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}
	srand((unsigned int)time(NULL));

	//start music:
	music = Sound::play(*(new Sound::Sample(data_path(filename))), 1.0f, 1.0f);
}

PlayMode::~PlayMode() {
	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_q || evt.key.keysym.sym == SDLK_KP_7) {
			pressed[0] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w || evt.key.keysym.sym == SDLK_KP_8) {
			pressed[1] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_e || evt.key.keysym.sym == SDLK_KP_9) {
			pressed[2] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_a || evt.key.keysym.sym == SDLK_KP_4) {
			pressed[3] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s || evt.key.keysym.sym == SDLK_KP_5) {
			pressed[4] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d || evt.key.keysym.sym == SDLK_KP_6) {
			pressed[5] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_z || evt.key.keysym.sym == SDLK_KP_1) {
			pressed[6] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_x || evt.key.keysym.sym == SDLK_KP_2) {
			pressed[7] = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_c || evt.key.keysym.sym == SDLK_KP_3) {
			pressed[8] = true;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		//std::cout << evt.motion.x << ", " << evt.motion.y << "\n";
		glm::vec2 clip_mouse = glm::vec2(
			(evt.motion.x + 0.5f) / window_size.x * 2.0f - 1.0f,
			(evt.motion.y + 0.5f) / window_size.y * -2.0f + 1.0f
		);
		glm::vec2 mouse_pos = (clip_to_court * glm::vec3(clip_mouse, 1.0f));
		int col = (int)((mouse_pos.x + playable_area_dim.x - offset) / (offset + 2 * button_dim.x));
		int row = (int)((mouse_pos.y - playable_area_dim.y + offset) / -(offset + 2 * button_dim.y));
		//std::cout << "col: " << col;
		//std::cout << " row: " << row << "\n";
		pressed[row * 3 + col] = true;
	}
	return false;
}

void PlayMode::update(float elapsed) {
	//update timer
	timer += elapsed;

	//debug
	/*
	while (timer >= beats.front()) {
		std::cout << beats.front() << "\n";
		beats.pop_front();
	}
	*/

	//add upcoming beats to active
	while (beats.size() > 0 && timer >= beats.front() - 0.6f) {
		//std::cout << "timer says: " << timer;
		//std::cout << "beat is: " << beats.front() << "\n";
		active_beats.push_back(beats.front());
		beats.pop_front();
		if (active_beats.size() > 9) {
			active_beats.pop_back();
			continue;
		}
		int rindex = rand() % keys.size(); //*select_randomly(keys.begin(), keys.end());
		keymap.push_back(keys[rindex]);
		keys.erase(keys.begin() + rindex);
	}

	/*
	std::cout << "Active beats: \n";
	for (uint32_t i = 0; i < active_beats.size(); ++i) {
		std::cout << active_beats[i] << " ,";
		std::cout << "Key: " << keymap[i] << "\n";
	}*/


	//remove old beats from active
	while (active_beats.size() > 0 && timer >= active_beats.front() + 0.6f) {
		if (keycolors[keymap.front()].z == 255) bad++; //if blue channel still exists
		keycolors[keymap.front()] = glm::u8vec4(255, 255, 255, 0); //reset color
		keys.push_back(keymap.front()); //return key to available
		keymap.pop_front();
		active_beats.pop_front();
	}

	//sanity
	if (active_beats.size() > 9) throw std::runtime_error("More than 9 active beats");;
	if (active_beats.size() != keymap.size()) throw std::runtime_error("Active beat size and keymap size do not match");
	if (keymap.size() + keys.size() != 9) throw std::runtime_error("Used keymaps and remaining keys don't sum to 9");;

	for (uint32_t i = 0; i < active_beats.size(); ++i) {
		int button_index = keymap[i];
		float diff = fabs(active_beats[i] - timer);
		if (pressed[button_index] && keycolors[button_index].z == 255) {
			if (diff < 0.2f) {
				perfect++;
				keycolors[button_index] = glm::u8vec4(0, 255, 0, keycolors[button_index].z);
			}
			else if (diff < 0.5f) {
				good++;
				keycolors[button_index] = glm::u8vec4(255, 255, 0, keycolors[button_index].z);
			}
			else {
				bad++;
				keycolors[button_index] = glm::u8vec4(255, 0, 0, keycolors[button_index].z);
			}
		}	
		keycolors[button_index] = glm::u8vec4(keycolors[button_index].x, keycolors[button_index].y, keycolors[button_index].z, 255 - (int)(diff/1.0f * 255));
	}
	score = 10 * perfect + 5 * good - 1 * bad;
	//reset button press counters:
	memset(pressed, false, 9);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 bg_color = glm::u8vec4(41, 23, 32, 255);
	const glm::u8vec4 fg_color = glm::u8vec4(255, 255, 255, 255);
	const glm::u8vec4 red = glm::u8vec4(255, 0, 0, 255);
	const glm::u8vec4 green = glm::u8vec4(0, 255, 0, 255);
	const glm::u8vec4 yellow = glm::u8vec4(255, 255, 0, 255);
	#undef HEX_TO_U8VEC4

	//other useful drawing constants:
	const float wall_radius = 0.05f;
	const float shadow_offset = 0.07f;
	const float padding = 0.14f; //padding between outside of walls and edge of window

	//---- compute vertices to draw ----

	//vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
	std::vector< Vertex > vertices;

	//inline helper function for rectangle drawing:
	//hooray rectangles
	auto draw_rectangle = [&vertices](glm::vec2 const& center, glm::vec2 const& radius, glm::u8vec4 const& color) {
		//draw rectangle as two CCW-oriented triangles:
		vertices.emplace_back(glm::vec3(center.x - radius.x, center.y - radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x + radius.x, center.y - radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x + radius.x, center.y + radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));

		vertices.emplace_back(glm::vec3(center.x - radius.x, center.y - radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x + radius.x, center.y + radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x - radius.x, center.y + radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
	};

	//solid objects:

	//walls:
	draw_rectangle(glm::vec2(-playable_area_dim.x - wall_radius, 0.0f), glm::vec2(wall_radius, playable_area_dim.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2(playable_area_dim.x + wall_radius, 0.0f), glm::vec2(wall_radius, playable_area_dim.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2(0.0f, -playable_area_dim.y - wall_radius), glm::vec2(playable_area_dim.x, wall_radius), fg_color);
	draw_rectangle(glm::vec2(0.0f, playable_area_dim.y + wall_radius), glm::vec2(playable_area_dim.x, wall_radius), fg_color);

	//buttons:
	for (uint32_t i = 0; i < 3; ++i) {
		for (uint32_t j = 0; j < 3; ++j) {
			draw_rectangle(glm::vec2(-playable_area_dim.x + button_dim.x + offset + ((offset + 2 * button_dim.x) * j), 
				playable_area_dim.y - button_dim.y - offset - ((offset + 2 * button_dim.y) * i)), 
				button_dim, keycolors[i * 3 + j]);
		}
	}

	//------ compute court-to-window transform ------

	//compute area that should be visible:
	glm::vec2 scene_min = glm::vec2(
		-playable_area_dim.x - 2.0f * wall_radius - padding,
		-playable_area_dim.y - 2.0f * wall_radius - padding
	);
	glm::vec2 scene_max = glm::vec2(
		playable_area_dim.x + 2.0f * wall_radius + padding,
		playable_area_dim.y + 2.0f * wall_radius + padding
	);

	//compute window aspect ratio:
	float aspect = drawable_size.x / float(drawable_size.y);
	//we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

	//compute scale factor for court given that...
	float scale = std::min(
		(2.0f * aspect) / (scene_max.x - scene_min.x), //... x must fit in [-aspect,aspect] ...
		(2.0f) / (scene_max.y - scene_min.y) //... y must fit in [-1,1].
	);

	glm::vec2 center = 0.5f * (scene_max + scene_min);

	//build matrix that scales and translates appropriately:
	glm::mat4 court_to_clip = glm::mat4(
		glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
	);
	//NOTE: glm matrices are specified in *Column-Major* order,
	// so each line above is specifying a *column* of the matrix(!)

	//also build the matrix that takes clip coordinates to court coordinates (used for mouse handling):
	clip_to_court = glm::mat3x2(
		glm::vec2(aspect / scale, 0.0f),
		glm::vec2(0.0f, 1.0f / scale),
		glm::vec2(center.x, center.y)
	);

	//---- actual drawing ----

	//clear the color buffer:
	glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.b / 255.0f, bg_color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	//upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set color_texture_program as current program:
	glUseProgram(color_texture_program.program);

	//upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

	//use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_texture_program);

	//bind the solid white texture to location zero so things will be drawn just with their colors:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	//run the OpenGL pipeline:
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

	//unbind the solid white texture:
	glBindTexture(GL_TEXTURE_2D, 0);

	//reset vertex array to none:
	glBindVertexArray(0);

	//reset current program to none:
	glUseProgram(0);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		lines.draw_text("Perfect: " + std::to_string(perfect),
			glm::vec3(-aspect + 0.1f * H, 1.0 - 1.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		lines.draw_text("Good: " + std::to_string(good),
			glm::vec3(-aspect + 0.1f * H, 1.0 - 2.2f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		lines.draw_text("Miss: " + std::to_string(bad),
			glm::vec3(-aspect + 0.1f * H, 1.0 - 3.3f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	}

	GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.

}

#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "Timer.h"

struct SDL_Texture;
struct j1UIElement;
struct j1UIScrollBar;
struct j1UILabel;

enum GUI_Event;

struct Level {
	p2SString map_path = "";
	p2SString sound_path = "";
	int next_level = 0;
	bool game_level = true;
};

struct Load_Data
{
	p2SString path;
	bool finished = false;
};

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool OnCollision(Collider* c1, Collider* c2);

	bool Load(pugi::xml_node &node);
	bool Save(pugi::xml_node &node) const;

	bool GUIEvent(j1UIElement* element, GUI_Event gui_event);

	void LoadLevel(bool load_save = false);
	void GameOver();

	int current_level = 0;
	j1UIElement* loading_background;
	j1UIElement* menu_background;
	j1UIElement* settings_menu_panel;
	j1UIElement* credits_menu_panel;
	j1UIElement* ingame_panel;
	uint32 saved_time = 0;
	j1UILabel* score_text;
	j1UILabel* time_text;
	Timer level_time;

private:
	bool game_running = true;

	p2List<Level> levels;

	j1UIElement* loading_animatedimage;
	j1UIElement* loading_panel;
	j1UIElement* loading_text;

	j1UIElement* website_button;
	j1UIElement* website_button_text;

	j1UIElement* main_menu_panel;
	j1UIElement* main_menu_button_play;
	j1UIElement* main_menu_button_continue;
	j1UIElement* main_menu_button_settings;
	j1UIElement* main_menu_button_credits;
	j1UIElement* main_menu_button_exit;

	j1UIElement* main_menu_button_play_text;
	j1UIElement* main_menu_button_continue_text;
	j1UIElement* main_menu_button_settings_text;
	j1UIElement* main_menu_button_credits_text;
	j1UIElement* main_menu_button_exit_text;

	j1UIElement* pause_menu_panel;
	j1UIElement* pause_menu_button_resume;
	j1UIElement* pause_menu_button_main_menu;
	j1UIScrollBar* pause_menu_music_slider;
	j1UIElement* pause_menu_music_text;
	j1UILabel* pause_menu_music_text_value;
	j1UIScrollBar* pause_menu_sfx_slider;
	j1UIElement* pause_menu_sfx_text;
	j1UILabel* pause_menu_sfx_text_value;
	
	j1UIElement* pause_menu_button_resume_text;
	j1UIElement* pause_menu_button_main_menu_text;


	j1UIElement* settings_menu_button_main_menu;
	j1UIElement* settings_menu_button_main_menu_text;
	j1UIScrollBar* settings_menu_music_slider;
	j1UIElement* settings_menu_music_text;
	j1UILabel* settings_menu_music_text_value;
	j1UIScrollBar* settings_menu_sfx_slider;
	j1UIElement* settings_menu_sfx_text;
	j1UILabel* settings_menu_sfx_text_value;


	j1UIElement* credits_menu_button_main_menu;
	j1UIElement* credits_menu_button_main_menu_text;
	j1UIElement* credits_menu_text_panel;
	j1UIElement* credits_menu_text;
	j1UIScrollBar* credits_menu_text_scroll;


	j1UILabel* lives_text;
	j1UIElement* lives_image;
	
	j1UIElement* score_image;
	

	
};

#endif // __j1SCENE_H__
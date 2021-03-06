#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1SwapScene.h"
#include "j1Map.h"
#include "j1Audio.h"
#include "j1Scene.h"
#include "j1EntityManager.h"
#include "j1Pathfinding.h"
#include "j1Collision.h"
#include "j1Gui.h"
#include "j1Enemy.h"
#include "Brofiler/Brofiler.h"

#define RIGHT_CAMERA_LIMIT  (-(App->render->camera.position.x - App->render->camera.body.w / 2))
#define LEFT_CAMERA_LIMIT  (-(App->render->camera.position.x - App->render->camera.body.w / 6))
#define TOP_CAMERA_LIMIT  (-(App->render->camera.position.y - App->render->camera.body.h / 8))
#define BOTTOM_CAMERA_LIMIT (-(App->render->camera.position.y - App->render->camera.body.h / 2))

j1Scene::j1Scene() : j1Module()
{
	name = "scene";
}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& conf)
{
	game_running = true;
	pugi::xml_node level;
	for (level = conf.child("levels").child("level"); level; level = level.next_sibling("level"))
	{
		Level lvl;
		lvl.map_path = level.child_value();
		lvl.sound_path = level.attribute("music").as_string();
		lvl.next_level = level.attribute("next_level").as_int();
		lvl.game_level = level.attribute("game_level").as_bool(true);
		levels.add(lvl);
	}
	current_level = conf.child("start_level").attribute("value").as_int();

	return true;
}

void j1Scene::LoadLevel(bool load_save)
{
	App->map->CleanUp();
	App->entitymanager->CleanMapEntities();

	App->map->Load(levels.At(current_level)->data.map_path.GetString());
	App->entitymanager->player->ResetEntity();

	if (levels.At(current_level)->data.game_level)
	{
		//pathfinding
		int w, h;
		uchar* data = NULL;
		if (App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);

		RELEASE_ARRAY(data);

		App->gui->EnableElement(ingame_panel);
		App->gui->DisableElement(pause_menu_panel);
		App->gui->DisableElement(loading_background);
		App->paused = false;
	}
	else
	{
		App->paused = true;
		App->gui->EnableElement(menu_background);
		App->gui->DisableElement(ingame_panel);
		App->gui->DisableElement(loading_background);
		App->gui->DisableElement(settings_menu_panel);
		App->gui->DisableElement(credits_menu_panel);
		App->gui->DisableElement(loading_background);
	}

	App->audio->PlayMusic(levels.At(current_level)->data.sound_path.GetString());

	if (load_save)
	{
		App->entitymanager->Load(App->current_save.child("game_state").child("entity_manager")); //TODO BETTER
		App->render->Load(App->current_save.child("game_state").child("renderer"));
	}

	level_time.Start();
}
// Called before the first frame
bool j1Scene::Start()
{
	BROFILER_CATEGORY("StartScene", Profiler::Color::Plum);
	uint x, y;
	App->win->GetWindowSize(x, y);
	ingame_panel = App->gui->CreateImage({ 0,0 }, { 0,0,(int)x,(int)y }, nullptr, false);
	menu_background = App->gui->CreateImage({ 0,0 }, { 1659, 2976,(int)x,(int)y });

	loading_background = App->gui->CreateImage({ 0,0 }, { 75, 2968, 1280, 720 });
	loading_panel = App->gui->CreateImage({ 450,420 }, { 1286, 2003, 357, 108 }, loading_background);
	App->gui->ScaleElement(loading_panel, 0.0F, 0.5F);
	loading_text = App->gui->CreateLabel({ 110,0 }, "fonts/open_sans/OpenSans-Bold.ttf", 28, "LOADING", { 255,255,255 }, 0, loading_panel);
	SDL_Rect rects[3] = {
		{ 444,3909,115,92 },
		{ 571,3903,118,92 },
		{ 698,3903,121,92 }
	};
	loading_animatedimage = App->gui->CreateAnimatedImage({120,30}, rects, 3, 10, loading_panel);

	website_button = App->gui->CreateButton({ 65,620 }, menu_background);
	App->gui->ScaleElement(website_button, -0.2F,-0.2F);
	website_button_text = App->gui->CreateLabel({ 30, 22 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "WEBSITE", { 255,255,255 }, 0, website_button);

	main_menu_panel = App->gui->CreateImage({ 850,50 }, { 551,711,380,539 }, menu_background);
	App->gui->ScaleElement(main_menu_panel, 0.0F, 0.17F);
	main_menu_button_play = App->gui->CreateButton({ 100, 52 }, main_menu_panel);
	main_menu_button_continue = App->gui->CreateButton({ 100, 137 }, main_menu_panel, App->save_file_exists);
	main_menu_button_settings = App->gui->CreateButton({ 100, 222 }, main_menu_panel);
	main_menu_button_credits = App->gui->CreateButton({ 100, 307 }, main_menu_panel);
	main_menu_button_exit = App->gui->CreateButton({ 100, 392 }, main_menu_panel);

	main_menu_button_play_text = App->gui->CreateLabel({ 58,22 }, "fonts/open_sans/OpenSans-Bold.ttf", 28, "PLAY", { 255,255,255 }, 0, main_menu_button_play);
	main_menu_button_continue_text = App->gui->CreateLabel({ 17,22 }, "fonts/open_sans/OpenSans-Bold.ttf", 28, "CONTINUE", { 255,255,255 }, 0, main_menu_button_continue);
	main_menu_button_settings_text = App->gui->CreateLabel({ 25,22 }, "fonts/open_sans/OpenSans-Bold.ttf", 28, "SETTINGS", { 255,255,255 }, 0, main_menu_button_settings);
	main_menu_button_credits_text = App->gui->CreateLabel({ 35,22 }, "fonts/open_sans/OpenSans-Bold.ttf", 28, "CREDITS", { 255,255,255 }, 0, main_menu_button_credits);
	main_menu_button_exit_text = App->gui->CreateLabel({ 60,22 }, "fonts/open_sans/OpenSans-Bold.ttf", 28, "EXIT", { 255,255,255 }, 0, main_menu_button_exit);

	settings_menu_panel = App->gui->CreateImage({ 450,50 }, { 551,711,380,539 }, menu_background);
	App->gui->ScaleElement(settings_menu_panel, 0.0F, -0.2F);
	settings_menu_button_main_menu = App->gui->CreateButton({ 100, 320 }, settings_menu_panel);
	settings_menu_button_main_menu_text = App->gui->CreateLabel({ 60,14 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "MAIN\nMENU", { 255,255,255 }, 100, settings_menu_button_main_menu);

	settings_menu_music_slider = credits_menu_text_scroll = App->gui->CreateScrollBar({ 110, 150 }, 0, MIX_MAX_VOLUME, HORIZONTAL, settings_menu_panel);
	settings_menu_music_slider->SetValue(App->audio->GetMusicVolume());
	settings_menu_music_text = App->gui->CreateLabel({ -55,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, "MUSIC", { 255,255,255 }, 0, settings_menu_music_slider);
	settings_menu_music_text->parent_limit = false;
	settings_menu_music_text->clipping = false;
	settings_menu_music_text_value = App->gui->CreateLabel({ 205,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, " ", { 255,255,255 }, 0, settings_menu_music_slider);
	settings_menu_music_text_value->parent_limit = false;
	settings_menu_music_text_value->clipping = false;

	settings_menu_sfx_slider = credits_menu_text_scroll = App->gui->CreateScrollBar({ 110, 250 }, 0, MIX_MAX_VOLUME, HORIZONTAL, settings_menu_panel);
	settings_menu_sfx_slider->SetValue(App->audio->GetFXVolume());
	settings_menu_sfx_text = App->gui->CreateLabel({ -55,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, "SFX", { 255,255,255 }, 0, settings_menu_sfx_slider);
	settings_menu_sfx_text->parent_limit = false;
	settings_menu_sfx_text->clipping = false;
	settings_menu_sfx_text_value = App->gui->CreateLabel({ 205,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, " ", { 255,255,255 }, 0, settings_menu_sfx_slider);
	settings_menu_sfx_text_value->parent_limit = false;
	settings_menu_sfx_text_value->clipping = false;

	pause_menu_panel = App->gui->CreateImage({ 450,50 }, { 551,711,380,539 }, ingame_panel);
	App->gui->ScaleElement(pause_menu_panel, 0.0F, -0.1F);
	pause_menu_button_resume = App->gui->CreateButton({ 100, 275 }, pause_menu_panel);
	pause_menu_button_main_menu = App->gui->CreateButton({ 100, 375 }, pause_menu_panel);
	pause_menu_button_resume_text = App->gui->CreateLabel({ 48,26 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "RESUME", { 255,255,255 }, 0, pause_menu_button_resume);
	pause_menu_button_main_menu_text = App->gui->CreateLabel({ 60,14 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "MAIN\nMENU", { 255,255,255 }, 100, pause_menu_button_main_menu);
	pause_menu_music_slider = credits_menu_text_scroll = App->gui->CreateScrollBar({ 120, 100 }, 0, MIX_MAX_VOLUME, HORIZONTAL, pause_menu_panel);
	pause_menu_music_slider->SetValue(App->audio->GetMusicVolume());
	pause_menu_music_text = App->gui->CreateLabel({ -55,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, "MUSIC", { 255,255,255 }, 0, pause_menu_music_slider);
	pause_menu_music_text->parent_limit = false;
	pause_menu_music_text->clipping = false;
	pause_menu_music_text_value = App->gui->CreateLabel({ 205,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, " ", { 255,255,255 }, 0, pause_menu_music_slider);
	pause_menu_music_text_value->parent_limit = false;
	pause_menu_music_text_value->clipping = false;
	pause_menu_sfx_slider = credits_menu_text_scroll = App->gui->CreateScrollBar({ 120, 200 }, 0, MIX_MAX_VOLUME, HORIZONTAL, pause_menu_panel);
	pause_menu_sfx_slider->SetValue(App->audio->GetFXVolume());
	pause_menu_sfx_text = App->gui->CreateLabel({ -55,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, "SFX", { 255,255,255 }, 0, pause_menu_sfx_slider);
	pause_menu_sfx_text->parent_limit = false;
	pause_menu_sfx_text->clipping = false;
	pause_menu_sfx_text_value = App->gui->CreateLabel({ 205,-4 }, "fonts/open_sans/OpenSans-Bold.ttf", 16, " ", { 255,255,255 }, 0, pause_menu_sfx_slider);
	pause_menu_sfx_text_value->parent_limit = false;
	pause_menu_sfx_text_value->clipping = false;

	credits_menu_panel = App->gui->CreateImage({ 450,50 }, { 551,711,380,539 }, menu_background);
	App->gui->ScaleElement(credits_menu_panel, 0.0F, 0.1F);
	credits_menu_text_panel = App->gui->CreateImage({ 60, 70 }, { 1172,2283,297,395 }, credits_menu_panel);
	App->gui->ScaleElement(credits_menu_text_panel, -0.12F, -0.1F);
	credits_menu_button_main_menu = App->gui->CreateButton({ 100, 400 }, credits_menu_panel);
	credits_menu_button_main_menu_text = App->gui->CreateLabel({ 60,14 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "MAIN\nMENU", { 255,255,255 }, 80, credits_menu_button_main_menu);
	credits_menu_text = App->gui->CreateLabel({ 5,5 }, "fonts/open_sans/OpenSans-Bold.ttf", 18,
		"MIT License\n\nCopyright(c) 2018 [Axel Alavedra Cabello, Alejandro Par�s G�mez]\n\nPermission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :\n\nThe above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.",
		{ 255,255,255 }, 250, credits_menu_text_panel);
	credits_menu_text->parent_limit = false;
	SDL_Rect parent_rect = credits_menu_text_panel->GetScreenRect();
	SDL_Rect screen_rect = credits_menu_text->GetScreenRect();
	credits_menu_text_scroll = App->gui->CreateScrollBar({ 330, 150 }, screen_rect.y, screen_rect.y - (screen_rect.h - parent_rect.h), VERTICAL, credits_menu_panel);

	lives_text = App->gui->CreateLabel({ 45,10 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "x 3", { 0,0,0 }, 80, ingame_panel);
	lives_image = App->gui->CreateImage({ 5,5 }, { 1974,1998,94,87 }, ingame_panel);
	App->gui->ScaleElement(lives_image, -0.6, -0.6F);

	time_text = App->gui->CreateLabel({ (ingame_panel->GetLocalRect().w / 2) - 20,10 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "00:00", { 0,0,0 }, 80, ingame_panel);

	score_text = App->gui->CreateLabel({ ingame_panel->GetLocalRect().w - 80,10 }, "fonts/open_sans/OpenSans-Bold.ttf", 22, "999", { 0,0,0 }, 80, ingame_panel);
	score_image = App->gui->CreateImage({ ingame_panel->GetLocalRect().w - 120,5 }, { 2253,1997,115,133 }, ingame_panel);
	App->gui->ScaleElement(score_image, -0.7F, -0.7F);

	App->gui->DisableElement(pause_menu_panel);
	App->gui->DisableElement(settings_menu_panel);
	App->gui->DisableElement(credits_menu_panel);
	App->gui->DisableElement(loading_background);
	App->gui->DisableElement(ingame_panel);

	LoadLevel();

	return true;
}

// Called each loop iteration
bool j1Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	//Debug Functionalities
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
	{
		saved_time = 0;
		App->entitymanager->player->ResetLives();
		current_level = 1;
		App->swap_scene->LoadScreen();
	}
	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
	{
		App->swap_scene->LoadScreen();
	}
	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
	{
		current_level = levels.At(current_level)->data.next_level;
		App->swap_scene->LoadScreen();
	}
	if (App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
	{
		App->gui->DisableElement(menu_background);
		App->gui->DisableElement(settings_menu_panel);
		App->gui->DisableElement(credits_menu_panel);
		App->gui->DisableElement(loading_background);
		App->LoadGame();
	}
	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
	{
		float scale = App->entitymanager->Reagroup();
		App->entitymanager->player->ScaleEntity(scale, scale);
		App->SaveGame();
	}
	if (App->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN && App->frame_cap == true)
	{
		App->frame_cap = false;
	}
	else if (App->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN)
	{
		App->frame_cap = true;
	}
	if (levels.At(current_level)->data.game_level && App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		if (App->paused)
		{
			App->paused = false;
			App->gui->DisableElement(pause_menu_panel);
		}
		else
		{
			App->gui->ScaleElement(pause_menu_panel, -0.5F, -0.5F);
			App->gui->ScaleElement(pause_menu_panel, 0.5F, 0.5F, 0.2F);
			App->paused = true;
			App->gui->EnableElement(pause_menu_panel);
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
	{
		App->entitymanager->DividePlayer();
	}
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
	{
		float scalex = App->entitymanager->Reagroup();
		App->entitymanager->player->ScaleEntity(scalex, scalex);
	}

	if (App->entitymanager->player->score > 4 && current_level == 1)
	{
		score_text->SetColor({ 0,255,0 });
	}
	else if (App->entitymanager->player->score > 4 && current_level == 2)
	{
		score_text->SetColor({ 0,255,0 });
	}
	
	App->render->camera.velocity = ((App->render->camera.target_speed * 0.4F) + (App->render->camera.velocity * (1 - 0.4F)));

	App->render->camera.position.y += App->render->camera.velocity.y;
	App->render->camera.position.x += App->render->camera.velocity.x;
	App->render->camera.body.x = App->render->camera.position.x;
	App->render->camera.body.y = App->render->camera.position.y;

	//Check if camera out of bounds and place it on right spot
	if (-(App->render->camera.position.y - App->render->camera.body.h) > App->map->data.height * App->map->data.tile_height)
		App->render->camera.position.y = -(App->map->data.height * App->map->data.tile_height - App->render->camera.body.h);
	else if (-App->render->camera.position.y < 0)
		App->render->camera.position.y = 0;
	if (-App->render->camera.position.x < 0)
		App->render->camera.position.x = 0;
	else if (-(App->render->camera.position.x - App->render->camera.body.w) > App->map->data.width*App->map->data.tile_width)
		App->render->camera.position.x = -(App->map->data.width * App->map->data.tile_width - App->render->camera.body.w);

	if(levels.At(current_level)->data.game_level)
		App->map->Draw();
	

	//SLIDER UPDATE
	BROFILER_CATEGORY("SLiderUpdates", Profiler::Color::Plum);
	credits_menu_text->SetScreenPos(credits_menu_text->GetScreenRect().x, credits_menu_text_scroll->GetValue());

	if (settings_menu_panel->enabled)
	{
		p2SString tmp_string;
		tmp_string.create("%i", (int)((settings_menu_music_slider->GetValue() / MIX_MAX_VOLUME) * 100));
		settings_menu_music_text_value->SetText(tmp_string);
		App->audio->SetMusicVolume((int)settings_menu_music_slider->GetValue());

		tmp_string.create("%i", (int)((settings_menu_sfx_slider->GetValue() / MIX_MAX_VOLUME) * 100));
		settings_menu_sfx_text_value->SetText(tmp_string);
		App->audio->SetFXVolume((int)settings_menu_sfx_slider->GetValue());
		
		pause_menu_music_slider->SetValue(App->audio->GetMusicVolume());
		pause_menu_sfx_slider->SetValue(App->audio->GetFXVolume());
	}
	else if (pause_menu_panel->enabled)
	{
		p2SString tmp_string;
		tmp_string.create("%i", (int)((pause_menu_music_slider->GetValue() / MIX_MAX_VOLUME) * 100));
		pause_menu_music_text_value->SetText(tmp_string);
		App->audio->SetMusicVolume((int)pause_menu_music_slider->GetValue());

		tmp_string.create("%i", (int)((pause_menu_sfx_slider->GetValue() / MIX_MAX_VOLUME) * 100));
		pause_menu_sfx_text_value->SetText(tmp_string);
		App->audio->SetFXVolume((int)pause_menu_sfx_slider->GetValue());

		settings_menu_music_slider->SetValue(App->audio->GetMusicVolume());
		settings_menu_sfx_slider->SetValue(App->audio->GetFXVolume());
	}
	else if (score_text->enabled)
	{
		p2SString tmp_string;
		tmp_string.create("%i", (int)((App->entitymanager->player->score)));
		score_text->SetText(tmp_string);

		tmp_string.create("x %i", (int)(App->entitymanager->player->lives));
		lives_text->SetText(tmp_string);


		tmp_string.create("%i", (int)(level_time.Read()/1000)+saved_time);
		time_text->SetText(tmp_string);
		if (((int)level_time.Read() / 1000) + saved_time >= 900)
		{
			time_text->SetColor({ 255,0,0 });
		}
		if (((int)level_time.Read() / 1000) + saved_time >= 1000)
		{
			current_level = 0;
			App->swap_scene->FadeToBlack(0.0F);

			App->gui->EnableElement(menu_background);
			App->gui->DisableElement(pause_menu_panel);



		}
	}

	if (main_menu_button_continue->interactable != App->save_file_exists)
		((j1UIButton*)main_menu_button_continue)->SetLocked(App->save_file_exists);

	return game_running;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;
	//Camera movement
	if (levels.At(current_level)->data.game_level)
	{
		if (App->entitymanager->player->position.x + App->entitymanager->player->collider->rect.w > RIGHT_CAMERA_LIMIT && App->entitymanager->player->velocity.x > 0.0F)
		{
			App->render->camera.target_speed.x = -(App->entitymanager->player->velocity.x);
		}
		else
		{
			if (App->entitymanager->player->position.x < LEFT_CAMERA_LIMIT && App->entitymanager->player->velocity.x < 0.0F)
			{
				App->render->camera.target_speed.x = -(App->entitymanager->player->velocity.x);
			}
			else
			{
				App->render->camera.target_speed.x = 0.0F;
			}
		}
		if (App->entitymanager->player->position.y < TOP_CAMERA_LIMIT && App->entitymanager->player->velocity.y < 0.0F)
		{
			App->render->camera.target_speed.y = -(App->entitymanager->player->velocity.y);
		}
		else
		{
			if (App->entitymanager->player->position.y + App->entitymanager->player->collider->rect.h > BOTTOM_CAMERA_LIMIT && App->entitymanager->player->velocity.y > 0.0F)
			{
				App->render->camera.target_speed.y = -(App->entitymanager->player->velocity.y);
			}
			else
			{
				App->render->camera.target_speed.y = 0.0F;
			}

		}
	}
	

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");
	App->map->CleanUp();
	levels.clear();

	loading_animatedimage = nullptr;
	loading_panel = nullptr;
	loading_text = nullptr;

	main_menu_panel = nullptr;
	main_menu_button_play = nullptr;
	main_menu_button_continue = nullptr;
	main_menu_button_settings = nullptr;
	main_menu_button_credits = nullptr;
	main_menu_button_exit = nullptr;

	main_menu_button_play_text = nullptr;
	main_menu_button_continue_text = nullptr;
	main_menu_button_settings_text = nullptr;
	main_menu_button_credits_text = nullptr;
	main_menu_button_exit_text = nullptr;

	pause_menu_panel = nullptr;
	pause_menu_button_resume = nullptr;
	pause_menu_button_main_menu = nullptr;
	pause_menu_music_slider = nullptr;
	pause_menu_music_text = nullptr;
	pause_menu_music_text_value = nullptr;
	pause_menu_sfx_slider = nullptr;
	pause_menu_sfx_text = nullptr;
	pause_menu_sfx_text_value = nullptr;

	pause_menu_button_resume_text = nullptr;
	pause_menu_button_main_menu_text = nullptr;

	settings_menu_panel = nullptr;
	settings_menu_button_main_menu = nullptr;
	settings_menu_button_main_menu_text = nullptr;
	settings_menu_music_slider = nullptr;
	settings_menu_music_text = nullptr;
	settings_menu_music_text_value = nullptr;
	settings_menu_sfx_slider = nullptr;
	settings_menu_sfx_text = nullptr;
	settings_menu_sfx_text_value = nullptr;

	credits_menu_panel = nullptr;
	credits_menu_button_main_menu = nullptr;
	credits_menu_button_main_menu_text = nullptr;
	credits_menu_text_panel = nullptr;
	credits_menu_text = nullptr;
	credits_menu_text_scroll = nullptr;

	ingame_panel = nullptr;
	lives_text = nullptr;
	lives_image = nullptr;
	time_text = nullptr;
	score_image = nullptr;
	score_text = nullptr;

	return true;
}

bool j1Scene::OnCollision(Collider * c1, Collider * c2)
{
	if (App->entitymanager->player->score > 4 && current_level == 1 || App->entitymanager->player->score > 4 && current_level == 2)
	{
		App->entitymanager->player->state = WIN;

		current_level = levels.At(current_level)->data.next_level;
		App->swap_scene->LoadScreen();
	}
	return true;
}

bool j1Scene::Load(pugi::xml_node &node)
{
	int saved_level = node.child("current_level").attribute("value").as_int();
	if (saved_level != current_level)
	{
		current_level = saved_level;
		App->swap_scene->LoadScreen(2.0F, true);
		if (levels.At(current_level)->data.game_level && menu_background->enabled)
			menu_background->enabled = false;
	}
	saved_time = node.child("time").attribute("value").as_uint(0);
	level_time.Start();

	return true;
}

bool j1Scene::Save(pugi::xml_node &node) const
{
	pugi::xml_node scene_node;
	scene_node = node.append_child("current_level");
	scene_node.append_attribute("value") = current_level;

	pugi::xml_node time_node = node.append_child("time");
	time_node.append_attribute("value") = level_time.Read() / 1000 + saved_time;
	

	return levels.At(current_level)->data.game_level?true:false;
}


void j1Scene::GameOver()
{
	App->gui->EnableElement(loading_background);
	current_level = 0;
	App->swap_scene->LoadScreen();
}

bool j1Scene::GUIEvent(j1UIElement * element, GUI_Event gui_event)
{
	switch (gui_event)
	{
		case MOUSE_OVER:
		{
			
		}
		break;
		case MOUSE_EXIT:
		{
		
		}
		break;
		case LEFT_CLICK_UP:
		{
			if (element == pause_menu_button_resume)
			{
				App->gui->DisableElement(pause_menu_panel);
				App->paused = false;
			}
			else if (element == settings_menu_button_main_menu)
			{
				App->gui->EnableElement(main_menu_panel);
				App->gui->DisableElement(settings_menu_panel);
			}
			else if(element == pause_menu_button_main_menu)
			{
				current_level = 0;
				App->gui->DisableElement(pause_menu_panel);
				App->swap_scene->LoadScreen();
			}
			else if (element == main_menu_button_play)
			{
				current_level = levels.At(current_level)->data.next_level;
				App->swap_scene->LoadScreen();
			}
			else if(element == main_menu_button_continue)
			{
				App->LoadGame();
				App->paused = false;
			}
			else if (element == main_menu_button_settings)
			{
				App->gui->ScaleElement(settings_menu_panel, -0.5f, -0.5f);
				App->gui->EnableElement(settings_menu_panel);
				App->gui->DisableElement(main_menu_panel);
				App->gui->ScaleElement(settings_menu_panel, 0.5f, 0.5f, 0.2F);
			}
			else if (element == main_menu_button_credits)
			{
				App->gui->ScaleElement(credits_menu_panel, -0.5f, -0.5f);
				App->gui->EnableElement(credits_menu_panel);
				App->gui->DisableElement(main_menu_panel);
				App->gui->ScaleElement(credits_menu_panel, 0.5f, 0.5f, 0.2F);
			}
			else if (element == main_menu_button_exit)
			{
				game_running = false;
			}
			else if (element == credits_menu_button_main_menu)
			{
				App->gui->EnableElement(main_menu_panel);
				App->gui->DisableElement(credits_menu_panel);
			}
			else if (element == settings_menu_sfx_slider->thumb || element == pause_menu_sfx_slider->thumb)
			{
				if(App->entitymanager->player->jump_fx) App->audio->PlayFx(App->entitymanager->player->jump_fx);
			}
			else if (element == website_button)
			{
				ShellExecuteA(NULL, "open", "https://thesaltmine.github.io/Development_Assignment/", NULL, NULL, SW_SHOWNORMAL);
			}
		}
		break;
	}

	return true;
}
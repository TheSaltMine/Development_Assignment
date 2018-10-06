#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Scene_Forest.h"
#include "j1Scene2.h"


j1Scene2::j1Scene2()
{
	name.create("scene2");
}

// Destructor
j1Scene2::~j1Scene2()
{}

// Called before render is available
bool j1Scene2::Awake(pugi::xml_node& conf)
{
	LOG("Loading Scene");
	map_file = conf.child("map_file").child_value();
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1Scene2::Start()
{
	App->map->Load(map_file.GetString());
	return true;
}

// Called each loop iteration
bool j1Scene2::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
		App->LoadGame();

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
		App->SaveGame();

	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		App->render->camera.y -= 1;

	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		App->render->camera.y += 1;

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		App->render->camera.x -= 1;

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		App->render->camera.x += 1;

	j1Scene::Update(dt);

	if (App->input->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) 
	{
		App->map->CleanUp();
		App->scene_forest->Activate();
		Deactivate();
	}

	return true;
}

bool j1Scene2::CleanUp()
{
	return true;
}
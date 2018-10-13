#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Map.h"
#include "j1Player.h"
#include "j1SwapScene.h"
#include "j1SceneForest.h"
#include "j1Audio.h"
#include "j1SceneMountain.h"
#include "j1Collision.h"


j1SceneMountain::j1SceneMountain()
{
	name.create("scene_mountain");
}

// Destructor
j1SceneMountain::~j1SceneMountain()
{}

// Called before render is available
bool j1SceneMountain::Awake(pugi::xml_node& conf)
{
	LOG("Loading Scene");
	map_file = conf.child("map_file").child_value();
	background_file = conf.child("background").child_value();
	music_file = conf.child("music").child_value();
	bool ret = true;

	return ret;
}

// Called each loop iteration
bool j1SceneMountain::Update(float dt)
{
	j1Scene::Update(dt);

	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) 
	{
		App->swap_scene->FadeToBlack(this, App->scene_forest);
	}

	return true;
}

bool j1SceneMountain::OnCollision(Collider* c1, Collider* c2)
{
	if (c2->type == COLLIDER_PLAYER && App->player->state != WIN)
	{
		App->player->state = WIN;
		App->swap_scene->FadeToBlack(this, App->scene_forest);
	}
	return true;
}
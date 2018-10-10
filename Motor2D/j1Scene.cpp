#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1SceneForest.h"
#include "j1SwapScene.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "j1Player.h"

#define HALF_CAMERA_WIDTH  (App->render->camera.w / 2)
#define QUARTER_CAMERA_WIDTH  (App->render->camera.w / 4)
#define QUARTER_CAMERA_HEIGHT  (App->render->camera.h / 4)

j1Scene::j1Scene() : j1Module()
{}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& conf)
{
	return true;
}

// Called before the first frame
bool j1Scene::Start()
{
	App->render->camera.y = (App->map->data.height*App->map->data.tile_height) - App->render->camera.h + App->render->camera.h / 6;
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
	if (App->player->position.x >= (-App->render->camera.x + HALF_CAMERA_WIDTH) && App->player->velocity.x > 0 && -App->render->camera.x < App->map->data.width*App->map->data.tile_width)
	{
		App->render->camera.x -= App->player->velocity.x;
		if (-App->render->camera.x > App->map->data.width*App->map->data.tile_width)
			App->render->camera.x = (-App->map->data.width*App->map->data.tile_width) - App->render->camera.w;
	}
	if (App->player->position.x <= (-App->render->camera.x + QUARTER_CAMERA_WIDTH) && App->player->velocity.x < 0 && -App->render->camera.x > 0)
	{
		App->render->camera.x += -App->player->velocity.x;
		if (-App->render->camera.x < 0)
			App->render->camera.x = 0;
	}

	if (App->player->position.y <= ((-App->render->camera.y + QUARTER_CAMERA_HEIGHT)) && App->player->velocity.y < 0 && -App->render->camera.y > 0)
	{
		App->render->camera.y -= App->player->velocity.y;
		if (-App->render->camera.y < 0)
			App->render->camera.y = 0;
	}
	if (App->player->position.y >= ((-App->render->camera.y)) && App->player->velocity.y > 0 && -App->render->camera.y < (App->map->data.height*App->map->data.tile_height)-App->render->camera.h - App->render->camera.h)
	{
		App->render->camera.y -= App->player->velocity.y;
		if (-App->render->camera.y > App->map->data.height*App->map->data.tile_height)
			App->render->camera.y = (-App->map->data.height*App->map->data.tile_height)-App->render->camera.h + App->render->camera.h;
	}

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

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) App->swap_scene->FadeToBlack(App->swap_scene->current_scene, App->scene_forest);
	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) App->swap_scene->Reload();

	App->map->Draw();

	p2SString title("Map:%dx%d Tiles:%dx%d Tilesets:%d",
					App->map->data.width, App->map->data.height,
					App->map->data.tile_width, App->map->data.tile_height,
					App->map->data.tilesets.count());

	App->win->SetTitle(title.GetString());
	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;

	if(App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}
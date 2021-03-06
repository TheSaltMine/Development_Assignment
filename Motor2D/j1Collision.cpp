#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "p2Log.h"
#include "j1Collision.h"

j1Collision::j1Collision()
{
	name.create("collision");
}

// Destructor
j1Collision::~j1Collision()
{}

bool j1Collision::Awake(pugi::xml_node& config) 
{
	max_colliders = config.child("max_colliders").attribute("value").as_int();
	colliders = new Collider*[max_colliders];
	for (uint i = 0; i < max_colliders; ++i)
		colliders[i] = nullptr;

	return true;
}
bool j1Collision::PreUpdate()
{
	// Remove colliders
	for (uint i = 0; i < max_colliders; ++i)
	{
		if (colliders[i] != nullptr && colliders[i]->to_delete == true)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	// Calculate collisions
		for (uint i = 0; i < max_colliders; ++i)
		{
			if (colliders[i] != nullptr && colliders[i]->type == COLLIDER_PLAYER)
			{
				player_collider = colliders[i];
			}
			// skip empty colliders
			if (colliders[i] != nullptr && colliders[i]->type != COLLIDER_PLAYER && colliders[i]->enabled)
			{
				if (player_collider->CheckCollision(colliders[i]->rect))
				{
					if (player_collider->callback)
						player_collider->callback->OnCollision(player_collider, colliders[i]);

					if (colliders[i]->callback)
						colliders[i]->callback->OnCollision(colliders[i], player_collider);
				}
			}
	}

	return true;
}

// Called before render is available
bool j1Collision::PostUpdate()
{
	DebugDraw();

	return true;
}

void j1Collision::DebugDraw()
{
	if (App->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		debug = !debug;

	if (debug == false)
		return;

	Uint8 alpha = 80;
	for (uint i = 0; i < max_colliders; ++i)
	{
		if (colliders[i] == nullptr)
			continue;

		switch (colliders[i]->type)
		{
		case COLLIDER_NONE: // white
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 255, alpha);
			break;
		case COLLIDER_PLATFORM: // blue
			App->render->DrawQuad(colliders[i]->rect, 0, 0, 255, alpha, true);
			break;
		case COLLIDER_FLOOR: // red
			App->render->DrawQuad(colliders[i]->rect, 255, 0, 0, alpha, true);
			break;
		case COLLIDER_PLAYER: // green
			App->render->DrawQuad(colliders[i]->rect, 0, 255, 0, alpha, true);
			break;
		case COLLIDER_TRIGGER: //yellow
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 0, alpha, true);
			break;
		case COLLIDER_ENEMY: //pink
			App->render->DrawQuad(colliders[i]->rect, 255, 0, 255, alpha, true);
			break;
		default:
			break;
		}
	}
}

// Called before quitting
bool j1Collision::CleanUp()
{
	LOG("Freeing all colliders");

	for (uint i = 0; i < max_colliders; ++i)
	{
		if (colliders[i] != nullptr)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}
	delete[] colliders;

	return true;
}

Collider* j1Collision::AddCollider(const SDL_Rect &rect, const COLLIDER_TYPE &type, j1Module* callback, const bool &player)
{
	Collider* ret = nullptr;

	for (uint i = 0; i < max_colliders; ++i)
	{
		if (colliders[i] == nullptr)
		{
			ret = colliders[i] = new Collider(rect, type, callback);
			if (player) player_collider = colliders[i];
			break;
		}
	}

	return ret;
}

// -----------------------------------------------------

bool Collider::CheckCollision(const SDL_Rect& r) const
{
	return ((r.x < rect.x + rect.w) && (rect.x < r.x + r.w)
		&& (r.y < rect.y + rect.h) && (rect.y < r.y + r.h));
}

// Looks for closest collider on right side and returns distance to it.
float j1Collision::DistanceToRightCollider(Collider* coll) const
{
	float distance = 999;

	for (uint i = 0; i < max_colliders; i++) 
	{
		if (colliders[i] != nullptr && colliders[i] != coll && colliders[i]->type == COLLIDER_FLOOR) //check for valid collider
		{
			if (colliders[i]->rect.x > coll->rect.x) //check for right side of received collider
			{
				if (coll->rect.y < colliders[i]->rect.y + colliders[i]->rect.h  && coll->rect.y + coll->rect.h > colliders[i]->rect.y) //possible collision
				{
					float new_distance = colliders[i]->rect.x - (coll->rect.x + coll->rect.w);
					if (new_distance < distance)
					{
						distance = new_distance;
					}	
				}
			}			
		}
	}
	return distance;
}

float j1Collision::DistanceToLeftCollider(Collider* coll) const
{
	float distance = -999;

	for (uint i = 0; i < max_colliders; i++)
	{
		if (colliders[i] != nullptr && colliders[i] != coll && colliders[i]->type == COLLIDER_FLOOR)
		{
			if (colliders[i]->rect.x < coll->rect.x)
			{
				if (coll->rect.y < colliders[i]->rect.y + colliders[i]->rect.h  && coll->rect.y + coll->rect.h > colliders[i]->rect.y)
				{
					float new_distance = (colliders[i]->rect.x + colliders[i]->rect.w) - coll->rect.x;
					if (new_distance > distance)
					{
						distance = new_distance;
					}
				}
			}
		}
	}
	return distance;
}

float j1Collision::DistanceToBottomCollider(Collider* coll, bool ignore_platform) const
{
	float distance = 999;

	for (uint i = 0; i < max_colliders; i++)
	{
		if (colliders[i] != nullptr && colliders[i] != coll && (ignore_platform? colliders[i]->type == COLLIDER_FLOOR:(colliders[i]->type == COLLIDER_PLATFORM || colliders[i]->type == COLLIDER_FLOOR)))
		{
			if (colliders[i]->rect.y >= coll->rect.y + coll->rect.h)
			{
				if (coll->rect.x < colliders[i]->rect.x + colliders[i]->rect.w  && coll->rect.x + coll->rect.w > colliders[i]->rect.x)
				{
					float new_distance = colliders[i]->rect.y - (coll->rect.y + coll->rect.h);
					if (new_distance < distance)
					{
						distance = new_distance;
					}
				}
			}
		}
	}
	return distance;
}

float j1Collision::DistanceToTopCollider(Collider* coll) const
{
	float distance = -999;

	for (uint i = 0; i < max_colliders; i++)
	{
		if (colliders[i] != nullptr && colliders[i] != coll && colliders[i]->type == COLLIDER_FLOOR)
		{
			if (colliders[i]->rect.y <= coll->rect.y)
			{
				if (coll->rect.x < colliders[i]->rect.x + colliders[i]->rect.w  && coll->rect.x + coll->rect.w > colliders[i]->rect.x)
				{
					float new_distance = (colliders[i]->rect.y + colliders[i]->rect.h) - coll->rect.y;
					if (new_distance > distance)
					{
						distance = new_distance;
					}
				}
			}
		}
	}
	return distance;
}
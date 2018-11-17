#ifndef __J1ENTITYMANAGER_H__
#define __J1ENTITYMANAGER_H__

#include "j1Module.h"
#include "p2DynArray.h"
#include "j1Entity.h"
#include "j1Player.h"


class j1EntityManager : public j1Module
{
public:

	p2List<j1Entity*> entities;
	j1Player* player;
	j1EntityManager();
	~j1EntityManager();

	bool Awake(pugi::xml_node&);
	bool Start();
	bool Update(float dt);
	bool PreUpdate();
	bool CleanUp();
	bool CreateEntity(EntityType type);
	j1Entity* getEntity(EntityType type);
	bool DeleteEntity(j1Entity* entity);

private:
	pugi::xml_node entity_configs;
};

#endif

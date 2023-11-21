#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

int Icomp::next_id = 0;     // class' static fields have to be initialized

int Entity::GetId() const { return id; }

void System::AddEntityToSystem(Entity entity) { entities.push_back(entity); }

void System::RemoveEntityFromSystem(Entity entity) 
{
	entities.erase(std::remove_if(entities.begin(),
		entities.end(),
		[&entity](Entity other) { return entity == other; } ),
		entities.end());
}

std::vector<Entity> System::GetSystemEntities() const { return entities; }

const Signature& System::GetComponentSignature() const { return comp_sign; }

Entity Registry::CreateEntity() {
	int entity_id = num_entities++;

	// if there are no free ids waiting to be reused
	if (entity_id >= entityComponentSignatures.size()) 
	{
		entityComponentSignatures.resize(entity_id + 1);
	}
	
	Entity ent(entity_id);
	ent.reg = this;
	entities_to_add.insert(ent);

	Logger::Log("Entity created with id " + std::to_string(entity_id));

	return ent;
}

void Registry::AddEntityToSystems(Entity ent) {
	const auto entity_id = ent.GetId();

	const auto& entityComponentSignature = entityComponentSignatures[entity_id];

	for (auto& system : systems) 
	{   // system is an iterator pointer not an object
		const auto& systemComponentSignature = system.second->GetComponentSignature();

		// "And" bitwise comparison between the two bitsets to see is the system component signature and the
		// entity component signature are compatible
		bool is_interested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;

		if (is_interested) 
		{
			system.second->AddEntityToSystem(ent);
		}
	}
}

void Registry::update() {
	// Here is where we actually insert/delete the entities that are waiting to be added/removed.
	// We do this because we don't want to confuse our Systems by adding/removing entities in the middle
	// of the frame logic. Therefore, we wait until the end of the frame to update and perform the
	// creation and deletion of entities.


	// Add the entities that are waiting to be created to the active Systems
	for (auto ent : entities_to_add) 
	{
		AddEntityToSystems(ent);
	}
	entities_to_add.clear();

	// Remove the entities that are waiting to be killed from the active Systems

}

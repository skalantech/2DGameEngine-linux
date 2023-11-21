#ifndef ECS_H
#define ECS_H

#include "../Logger/Logger.h"
#include <vector>
#include <bitset>
#include <set>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <iostream>


const unsigned int MAX_COMPS = 32;

////////////////////////////////////////////////////////////////////////////////
// Signature
////////////////////////////////////////////////////////////////////////////////
// We use a bitset (1s and 0s) to keep track of which componentities an entity has,
// and also helps keep track of which entities a system is interested in.
////////////////////////////////////////////////////////////////////////////////
typedef std::bitset<MAX_COMPS> Signature;

struct Icomp 
{
protected:
	static int next_id;
};

// Used to assign a unique id to a component type
template <typename T>       // Placeholder for the several different classes that will be created in compilation
class Component : public Icomp 
{
public:
	// Returns the unique id of Component<T>
	static int GetId() {
		static auto id = next_id++;
		return id;
	}
};

////////////////////////////////////////////////////////////////////////////////
// Entity
////////////////////////////////////////////////////////////////////////////////
class Entity 
{
private:
	int id;

public:
	Entity(int id) : id(id) {};
	Entity(const Entity& ent) = default;  // copy constructor
	int GetId() const;

	// Manage entity tags and groups


	// Operator overloading for entity objects
	Entity& operator =(const Entity& other) = default;      // copy assignment
	bool operator ==(const Entity& other) const { return id == other.id; }
	bool operator !=(const Entity& other) const { return id != other.id; }
	bool operator >(const Entity& other) const { return id > other.id; }
	bool operator <(const Entity& other) const { return id < other.id; }

	// Manage entity componentities
	template <typename Tcomp, typename ...Targs> void AddComponent(Targs&& ...args);
	template <typename Tcomp> void RemoveComponent();
	template <typename Tcomp> bool HasComponent() const;
	template <typename Tcomp> Tcomp& GetComponent() const;


	// Hold a pointer to the entity's owner registry
	class Registry* reg{};
};

////////////////////////////////////////////////////////////////////////////////
// System
////////////////////////////////////////////////////////////////////////////////
// The system processes entities that contain a specific signature
////////////////////////////////////////////////////////////////////////////////
class System 
{
private:
	Signature comp_sign;
	std::vector<Entity> entities;

public:
	System() = default;
	~System() = default;

	void AddEntityToSystem(Entity ent);
	void RemoveEntityFromSystem(Entity ent);
	std::vector<Entity> GetSystemEntities() const;
	const Signature& GetComponentSignature() const;

	// Defines the component type that entities must have to be considered by the system
	template <typename Tcomp> void RequireComponent();
};

////////////////////////////////////////////////////////////////////////////////
// Pool
////////////////////////////////////////////////////////////////////////////////
// A pool is just a vector (contiguous data) of objects of type T
////////////////////////////////////////////////////////////////////////////////
class Ipool 
{
public:
	virtual ~Ipool() = default;
};

template <typename T>
class Pool : public Ipool 
{
private:
	// We keep track of the vector of objects and the current number of elementities
	std::vector<T> data;

	// Helper maps to keep track of entity ids per index, so the vector is always packed

public:
	Pool(int cap = 100)	{ data.resize(cap); }

	virtual ~Pool() = default;

	bool is_empty() const {	return data.empty(); }

	int get_size() const { return data.size();	}

	void resize(int n) { data.resize(n); }

	void clear() { data.clear(); }

	void add(T obj) { data.push_back(obj); }

	void set(int idx, T obj) { data[idx] = obj;	}

	T& get(int idx) { return static_cast<T&>(data[idx]); }

	T& operator [](unsigned int idx) { return data[idx]; }
};

////////////////////////////////////////////////////////////////////////////////
// Registry
////////////////////////////////////////////////////////////////////////////////
// The registry manages the creation and destruction of entities, add systems,
// and componentities.
////////////////////////////////////////////////////////////////////////////////
class Registry 
{
private:
	int num_entities = 0;

	// Vector of component pools, each pool contains all the data for a certain compoenent type
	// [Vector index = component type id]
	// [Pool index = entity id]
	std::vector<std::shared_ptr<Ipool>> comp_pools;     // creating IPool so that we don't have to specify the type of Pool (Polymorphism)

	// Vector of component signatures per entity, saying which component is turned "on" for a given entity
	// [Vector index = entity id]
	std::vector<Signature> entityComponentSignatures;

	// Map of active systems
	// [Map key = system type id]
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

	// Set of entities that are flagged to be added or removed in the next registry Update()
	std::set<Entity> entities_to_add;
	std::set<Entity> entities_to_kill;

	// Entity tags (one tag name per entity)


	// Entity groups (a set of entities per group name)


	// List of free entity ids that were previously removed


public:
	Registry() { Logger::Log("Registry constructor called"); }

	~Registry() { Logger::Log("Registry destructor called"); }

	// The registry Update() finally processes the entities that are waiting to be added/killed to the systems
	void update();

	// Entity management
	Entity CreateEntity();


	// Tag management


	// Group management


	// Component management
	template <typename Tcomp, typename ...Targs> void AddComponent(Entity ent, Targs&& ...args);
	template <typename Tcomp> void RemoveComponent(Entity ent);
	template <typename Tcomp> bool HasComponent(Entity ent) const;
	template <typename Tcomp> Tcomp& GetComponent(Entity ent) const;

	// System management
	template <typename Tsys, typename ...Targs> void AddSystem(Targs&& ...args);
	template <typename Tsys> void RemoveSystem();
	template <typename Tsys> bool HasSystem() const;
	template <typename Tsys> Tsys& GetSystem() const;

	// Checks the component signature of an entity and add or remove the entity to the systems
	// that are interested in it
	void AddEntityToSystems(Entity ent);
};

// Template function implementation
// Template functions are not real functions yet, they are placeholder for the functions of different types
// that the compiler is going to create after compilation

// System
template <typename Tcomp>
void System::RequireComponent() 
{
	const auto comp_id = Component<Tcomp>::GetId();
	comp_sign.set(comp_id);
}

// Registry
template <typename Tsys, typename ...Targs>
void Registry::AddSystem(Targs&& ...args) 
{
	std::shared_ptr<Tsys> new_sys = std::make_shared<Tsys>(std::forward<Targs>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(Tsys)), new_sys));
}

template <typename Tsys>
void Registry::RemoveSystem() 
{
	auto system = systems.find(std::type_index(typeid(Tsys))); // system is an iterator pointer not an object
	systems.erase(system);
}

template <typename Tsys>
bool Registry::HasSystem() const 
{
	return systems.find(std::type_index(typeid(Tsys))) != systems.end();
}

template <typename Tsys>
Tsys& Registry::GetSystem() const 
{
	auto system = systems.find(std::type_index(typeid(Tsys)));
	return *(std::static_pointer_cast<Tsys>(system->second));
}

template <typename Tcomp, typename ...Targs>
void Registry::AddComponent(Entity ent, Targs&& ...args) 
{
	const auto comp_id = Component<Tcomp>::GetId();
	const auto entity_id = ent.GetId();

	// If the component id is greater than the current size of the componentPools, then resize the vector
	if (comp_id >= comp_pools.size()) 
	{
		comp_pools.resize(comp_id + 1, nullptr);    // resizing a vector is very costly it is better to allocate more memory from the getgo!
	}

	// If we still don't have a Pool for that component type
	if (!comp_pools[comp_id]) 
	{
		std::shared_ptr<Pool<Tcomp>> new_comp_pool(new Pool<Tcomp>()); // = std::make_shared<Pool<Tcomp>>();
		comp_pools[comp_id] = new_comp_pool;
	}

	// Get the pool of component values for that component type
	std::shared_ptr<Pool<Tcomp>> comp_pool = std::static_pointer_cast<Pool<Tcomp>>(comp_pools[comp_id]);

	// If the entity id is greater than the current size of the component pool, then resize the pool
	if (entity_id >= comp_pool->get_size()) 
	{
		comp_pool->resize(num_entities);
	}

	// Create a new Component object of the type T, and forward the various parameters to the constructor
	Tcomp new_comp(std::forward<Targs>(args)...);

	// Add the new component to the component pool list, using the entity id as index
	comp_pool->set(entity_id, new_comp);

	// Finally, change the component signature of the entity and set the component id on the bitset to 1
	entityComponentSignatures[entity_id].set(comp_id);

	Logger::Log("Component id = " + std::to_string(comp_id) + " was added to entity id " + std::to_string(entity_id));

	//std::cout << "COMPONENT ID " << comp_id << " --> POOL SIZE: " << comp_pool->get_size() << std::endl;
}

template <typename Tcomp>
void Registry::RemoveComponent(Entity ent) 
{
	const auto comp_id = Component<Tcomp>::GetId();
	const auto entity_id = ent.GetId();

	// Remove the component from the component list for that entity

	//Set this component signature for that entity to false
	entityComponentSignatures[entity_id].set(comp_id, false);

	Logger::Log("Component id = " + std::to_string(comp_id) + " was removed from entity id " + std::to_string(entity_id));
}

template <typename Tcomp> 
bool Registry::HasComponent(Entity ent) const
{
	const auto comp_id = Component<Tcomp>::GetId();
	const auto entity_id = ent.GetId();
	return entityComponentSignatures[entity_id].test(comp_id);
}

template <typename Tcomp>
Tcomp& Registry::GetComponent(Entity ent) const {
	const auto comp_id = Component<Tcomp>::GetId();
	const auto entity_id = ent.GetId();
	auto comp_pool = std::static_pointer_cast<Pool<Tcomp>>(comp_pools[comp_id]);
	return comp_pool->get(entity_id);
}

// Entity
template <typename Tcomp, typename ...Targs>
void Entity::AddComponent(Targs&& ...args) {
	reg->AddComponent<Tcomp>(*this, std::forward<Targs>(args)...);
}

template <typename Tcomp>
void Entity::RemoveComponent() {
	reg->RemoveComponent<Tcomp>(*this);
}

template <typename Tcomp>
bool Entity::HasComponent() const {
	return reg->HasComponent<Tcomp>(*this);
}

template <typename Tcomp>
Tcomp& Entity::GetComponent() const {
	return reg->GetComponent<Tcomp>(*this);
}

#endif

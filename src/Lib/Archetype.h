#pragma once
#include <Base/Always.h>
#include <vector>
#include <unordered_map>
#include <Memory.h>

// Entities, types and entity index
using EntityId = uint64_t;
using Type = std::vector<EntityId>;
std::unordered_map<EntityId, Type> entity_index;// Type flags
const EntityId INSTANCEOF = 1 << 63;
const EntityId CHILDOF = 2 << 62;// Component data

struct ComponentArray
{
	void* elements; // vector<T>
	int size;
};

// Archetype graph
struct Archetype; 

struct Edge
{
	std::shared_ptr<Archetype> add;
	std::shared_ptr<Archetype> remove;
};

struct Archetype
{
	Type type;
	std::vector<EntityId> entity_ids;
	std::vector<ComponentArray> components;
	int length;
	std::vector<Edge> edges;

	Archetype* node = root;
	for (int i = 0; i < type.size(); i++)
	{
		Edge* edge = &node->edges[type[i]];
		if (!edge->add)
		{
			edge->add = create_archetype(node, type[i]);
		}

		node = edge->add;
	}
};


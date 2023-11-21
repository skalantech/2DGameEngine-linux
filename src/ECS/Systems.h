#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "../ECS/ECS.h"
#include "../ECS/Components.h"
#include "../AssetStore/AssetStore.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


class MovementSystem : public System 
{
public:
    MovementSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void update(double dt) 
    {
        // Loop all entities that the system is interested in
        for (auto entity : GetSystemEntities()) {
            // Update entity position based on its velocity
            auto& transform = entity.GetComponent<TransformComponent>();
            const auto rigidbody = entity.GetComponent<RigidBodyComponent>();

            transform.pos.x += rigidbody.vel.x * dt;
            transform.pos.y += rigidbody.vel.y * dt;
        }
    }
};

class RenderSystem : public System 
{
public:
    RenderSystem() 
    {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    void update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& asset_store) {
        // Loop all entities that the system is interested in
        for (auto entity : GetSystemEntities()) {
            const auto& transform = entity.GetComponent<TransformComponent>();
            const auto sprite = entity.GetComponent<SpriteComponent>();



            // Set the source rectangle of our original sprite texture
            SDL_Rect src_rect = sprite.src_rect;

            // Set the destination rectangle of
            SDL_Rect dst_rect = {
                static_cast<int>(transform.pos.x),
                static_cast<int>(transform.pos.y),
                static_cast<int>(sprite.width * transform.scale.x),
                static_cast<int>(sprite.height * transform.scale.y)
            };

            SDL_RenderCopyEx(renderer, asset_store->get_texture(sprite.asset_id), &src_rect, &dst_rect, transform.rot, NULL, SDL_FLIP_NONE);

            //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            //SDL_RenderFillRect(renderer, &obj_rect);

            // Draw a PNG texture
        }
    }
};

#endif
#include "figure_constable.h"

#include "city/city.h"
#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/service.h"
#include "figuretype/figure_native.h"
#include "figuretype/figure_tomb_robber.h"
#include "game/game_config.h"
#include "grid/road_access.h"
#include "grid/routing/routing.h"
#include "js/js_game.h"

#include <algorithm>

REPLICATE_STATIC_PARAMS_FROM_CONFIG(figure_constable);

void figure_constable::figure_before_action() {
    building* h = home();
    if (h->state != BUILDING_STATE_VALID || !h->has_figure(0, id())) {
        poof();
    }
}

void figure_constable::figure_action() {
    law_walker_action(true);
}

tile2i figure_constable::home_road() const {
    const building* h = home();
    return map_closest_road_within_radius(h->tile, h->size, current_params().home_road_radius);
}

void figure_constable::acquire_attack() {
    base.set_flag(e_figure_flag_inattack);
}

void figure_constable::law_walker_action(bool is_constable) {
    base.terrain_usage = TERRAIN_USAGE_ROADS;

    auto& d = runtime_data();
    const bool wrapped = ++d.walk_frame >= LAW_WALKER_ANIMATION_FRAMES;
    if (wrapped) {
        d.walk_frame = 0;
    }
    if (wrapped && is_constable) {
        fight_enemy();
    }
    if (is_constable) {
        arrest_tomb_robber_remake();
    }

    if (base.in_attack()) {
        if (is_constable && base.attack_image_offset == 12) {
            base.play_attack_sound();
        }
        base.figure_combat_handle_attack();
        return;
    }

    switch (action_state()) {
    case FIGURE_ACTION_149_CORPSE:
        break;

    case ACTION_76_CONSTABLE_CHASE:
        law_walker_chase();
        break;

    case ACTION_70_CONSTABLE_CREATED: {
        d.walk_frame = 0;
        if (--base.wait_ticks > 0) {
            break;
        }
        if (!home_road().valid()) {
            poof();
            break;
        }
        advance_action(ACTION_72_CONSTABLE_ROAMING);
        base.roam_length = 0;
        break;
    }

    case ACTION_71_CONSTABLE_ENTERING_EXITING:
        do_enterbuilding(true, home());
        break;

    case ACTION_72_CONSTABLE_ROAMING:
        if (++base.roam_length >= base.max_roam_length) {
            const tile2i road = home_road();
            if (!road.valid()) {
                poof();
            } else {
                advance_action(ACTION_73_CONSTABLE_RETURNING);
                base.destination_tile = road;
                route_remove();
            }
        }
        base.roam_ticks(base.speed_multiplier);
        break;

    case ACTION_73_CONSTABLE_RETURNING:
        base.move_ticks(base.speed_multiplier);
        if (direction() == DIR_FIGURE_REROUTE || direction() == DIR_FIGURE_CAN_NOT_REACH) {
            poof();
        } else if (direction() == DIR_FIGURE_NONE) {
            advance_action(ACTION_71_CONSTABLE_ENTERING_EXITING);
            base.destination_tile = home()->tile;
            base.roam_length = 0;
        }
        break;

    default:
        advance_action(ACTION_70_CONSTABLE_CREATED);
        break;
    }
}

void figure_constable::law_walker_chase() {
    base.terrain_usage = TERRAIN_USAGE_ANY;
    if (!base.target_is_alive()) {
        const tile2i road = home_road();
        if (!road.valid()) {
            poof();
        } else {
            advance_action(ACTION_73_CONSTABLE_RETURNING);
            base.destination_tile = road;
            route_remove();
            base.roam_length = 0;
        }
    }

    base.move_ticks(base.speed_multiplier);
    if (direction() == DIR_FIGURE_REROUTE || direction() == DIR_FIGURE_CAN_NOT_REACH) {
        poof();
    } else if (direction() == DIR_FIGURE_NONE) {
        base.destination_tile = figure_get(base.target_figure_id)->tile;
        route_remove();
    }
}

bool figure_constable::fight_enemy() {
    if (!g_city.figures_has_security_breach() && enemy_army_total_enemy_formations() <= 0) {
        return false;
    }

    if (base.in_attack()) {
        return false;
    }

    switch (action_state()) {
    case FIGURE_ACTION_149_CORPSE:
    case ACTION_70_CONSTABLE_CREATED:
    case ACTION_71_CONSTABLE_ENTERING_EXITING:
    case ACTION_76_CONSTABLE_CHASE:
        return false;
    }

    const auto& p = current_params();
    auto& d = runtime_data();
    if (++d.wait_ticks_next_target < p.enemy_search_ticks) {
        return false;
    }

    d.wait_ticks_next_target = 0;
    const figure_id enemy_id = closest_enemy(p.enemy_search_distance);
    if (enemy_id <= 0) {
        return false;
    }

    figure* enemy = figure_get(enemy_id);
    advance_action(ACTION_76_CONSTABLE_CHASE);
    base.destination_tile = enemy->tile;
    base.target_figure_id = enemy_id;
    enemy->targeted_by_figure_id = id();
    route_remove();
    return true;
}

// Remake tomb robber (gameplay_change_tomb_robber_remake) is caught by a constable; OG constables only fight on
// contact.
void figure_constable::arrest_tomb_robber_remake() {
    if (!game_features::gameplay_change_tomb_robber_remake.to_bool() || base.in_attack()) {
        return;
    }

    switch (action_state()) {
    case FIGURE_ACTION_149_CORPSE:
    case ACTION_70_CONSTABLE_CREATED:
    case ACTION_71_CONSTABLE_ENTERING_EXITING:
    case ACTION_76_CONSTABLE_CHASE:
        return;
    }

    const int max_distance = current_params().tomb_robber_arrest_distance;
    const auto result = base.is_nearby(NEARBY_HOSTILE, max_distance);
    if (result.fid <= 0 || result.distance > max_distance) {
        return;
    }

    figure* criminal = figure_get(result.fid);
    if (criminal->type != FIGURE_TOMB_ROBER || !criminal->is_alive()) {
        return;
    }

    auto* robber = criminal->dcast<figure_tomb_robber>();
    if (robber && robber->arrest()) {
        base.target_figure_id = 0;
    }
}

figure_id figure_constable::closest_enemy(int max_distance) const {
    const auto& p = current_params();
    std::array<figure_id, LAW_WALKER_MAX_ENEMY_CANDIDATES> ids;
    std::array<int, LAW_WALKER_MAX_ENEMY_CANDIDATES> offsets;
    int count = 0;

    for (int i = 1; i < MAX_FIGURES; i++) {
        figure* f = figure_get(i);
        if (!f->is_alive() || f->targeted_by_figure_id != 0) {
            continue;
        }
        if (count >= LAW_WALKER_MAX_ENEMY_CANDIDATES) {
            break;
        }

        int factor = 0;
        if (f->type == FIGURE_INDIGENOUS_NATIVE && f->action_state == ACTION_11_NATIVE_ATTACKING) {
            factor = p.native_distance_factor;
        } else if (f->dcast()->dcast_enemy() && !f->is_boat()) {
            factor = p.enemy_distance_factor;
        } else if (std::find(p.chased_animals.begin(), p.chased_animals.end(), f->type) != p.chased_animals.end()) {
            factor = p.animal_distance_factor;
        } else {
            continue;
        }

        const int distance = calc_maximum_distance(tile(), f->tile) * factor;
        if (distance < max_distance) {
            ids[count] = i;
            offsets[count] = f->tile.grid_offset();
            count++;
        }
    }

    if (count == 0) {
        return 0;
    }

    const int index = map_routing_citizen_find_first_of_any(tile(), offsets.data(), count);
    return index > 0 ? ids[index - 1] : 0;
}

int figure_constable::provide_service() {
    base.min_max_seen = (uint8_t)figure_provide_crime_coverage(tile(), current_params().crime_reduction);
    return 0;
}

void figure_constable::update_animation() {
    figure_impl::update_animation();

    xstring anim_key = animkeys().walk;
    if (action_state() == FIGURE_ACTION_149_CORPSE) {
        anim_key = animkeys().death;
    } else if (base.in_attack()) {
        anim_key = animkeys().attack;
    }

    image_set_animation(anim_key);
}

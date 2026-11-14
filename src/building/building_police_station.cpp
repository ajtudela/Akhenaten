#include "building_police_station.h"

#include "figuretype/figure_constable.h"
#include "js/js_game.h"

REPLICATE_STATIC_PARAMS_FROM_CONFIG(building_police_station);

void building_police_station::spawn_figure() {
    check_labor_problem();
    if (has_figure(BUILDING_SLOT_SERVICE) || !has_road_access()) {
        return;
    }

    common_spawn_labor_seeker(current_params().min_houses_coverage);

    const int spawn_delay = figure_spawn_timer();
    if (spawn_delay == -1) {
        return;
    }

    base.figure_spawn_delay++;
    if (base.figure_spawn_delay > spawn_delay) {
        base.figure_spawn_delay = 0;
        create_roaming_figure(FIGURE_CONSTABLE, (e_figure_action)ACTION_70_CONSTABLE_CREATED, BUILDING_SLOT_SERVICE);
    }
}

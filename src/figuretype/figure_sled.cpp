#include "figure_sled.h"

#include "core/profiler.h"
#include "js/js_game.h"

REPLICATE_STATIC_PARAMS_FROM_CONFIG(figure_sled);
REPLICATE_STATIC_PARAMS_FROM_CONFIG(figure_sled_puller);

void figure_sled::figure_action() {
    OZZY_PROFILER_FUNCTION();
    pyramid_figure_train_follow(id(), 1);
}

void figure_sled::update_animation() {
    xstring animkey;
    switch (base.resource_id) {
    case RESOURCE_STONE: animkey = "stone"; break;
    case RESOURCE_LIMESTONE: animkey = "limestone"; break;
    case RESOURCE_GRANITE: animkey = "granite"; break;
    case RESOURCE_SANDSTONE: animkey = "sandstone"; break;
    case RESOURCE_MARBLE: animkey = "marble"; break;
    case RESOURCE_COPPER: animkey = "copper"; break;
    case RESOURCE_CLAY: animkey = "clay"; break;
    case RESOURCE_BRICKS: animkey = "bricks"; break;

    default:
        animkey = "empty";
        break;
    }

    image_set_animation(animkey);
}

void figure_sled_puller::figure_action() {
    OZZY_PROFILER_FUNCTION();
    pyramid_figure_train_follow(id(), 0);
}

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
    const auto& p = current_params();
    const resource_image_t* image = &p.default_image;
    for (const auto& entry : p.resource_images) {
        if (entry.resource == base.resource_id) {
            image = &entry;
            break;
        }
    }

    const auto& rd = runtime_data();
    const bool on_ramp = rd.walk_mode != PYRAMID_WALK_NONE && rd.climb.previous_height < rd.climb.height;
    image_set_animation(on_ramp ? image->ramp : image->ground);
}

void figure_sled_puller::figure_action() {
    OZZY_PROFILER_FUNCTION();
    pyramid_figure_train_follow(id(), 0);
}

void figure_sled_puller::update_animation() {
    image_set_animation(runtime_data().side ? animkeys().pulling : animkeys().front);
}

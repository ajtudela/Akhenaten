#pragma once

#include "building/monument_pyramid_figure.h"
#include "figuretype/figure_cartpusher.h"

enum e_sled_action {
    // OG FIGURE_ACTION_LABORER_IDLE — train members stay here while following.
    ACTION_8_SLED_TRAIN_FOLLOW = 8,
};

class figure_sled : public figure_carrier {
public:
    FIGURE_METAINFO(FIGURE_SLED, figure_sled)
    figure_sled(figure *f) : figure_carrier(f) {}

    struct runtime_data_t {
        uint8_t walk_mode;
        pyramid_figure_work_t climb;
    } FIGURE_RUNTIME_DATA_T;

    virtual void figure_action() override;
    virtual void update_animation() override;
    virtual figure_sled *dcast_sled() override { return this; }
};

class figure_sled_puller : public figure_impl {
public:
    FIGURE_METAINFO(FIGURE_SLED_PULLER, figure_sled_puller)
    figure_sled_puller(figure *f) : figure_impl(f) {}

    struct runtime_data_t {
        uint8_t walk_mode;
        pyramid_figure_work_t climb;
    } FIGURE_RUNTIME_DATA_T;

    virtual void figure_action() override;
    virtual figure_sled_puller *dcast_sled_puller() override { return this; }
};

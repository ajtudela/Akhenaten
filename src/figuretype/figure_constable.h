#pragma once

#include "figure/figure.h"

enum e_constable_action {
    ACTION_70_CONSTABLE_CREATED = 70,
    ACTION_71_CONSTABLE_ENTERING_EXITING = 71,
    ACTION_72_CONSTABLE_ROAMING = 72,
    ACTION_73_CONSTABLE_RETURNING = 73,
    ACTION_76_CONSTABLE_CHASE = 76,
};

enum e_law_walker_limits {
    LAW_WALKER_ANIMATION_FRAMES = 12,
    LAW_WALKER_MAX_ENEMY_CANDIDATES = 100,
};

class figure_constable : public figure_impl {
public:
    FIGURE_METAINFO(FIGURE_CONSTABLE, figure_constable)
    figure_constable(figure* f) : figure_impl(f) {}

    struct static_params : public figure_static_params {
        int home_road_radius;
        int crime_reduction;
        int enemy_search_ticks;
        int enemy_search_distance;
        int native_distance_factor;
        int enemy_distance_factor;
        int animal_distance_factor;
        svector<e_figure_type, 16> chased_animals;
        int tomb_robber_arrest_distance;
    } FIGURE_STATIC_DATA_T;

    struct runtime_data_t {
        uint8_t wait_ticks_next_target;
        uint8_t walk_frame;
    } FIGURE_RUNTIME_DATA_T;

    virtual void on_create() override {}
    virtual void figure_action() override;
    virtual void figure_before_action() override;
    virtual void update_animation() override;
    virtual int provide_service() override;
    virtual void acquire_attack() override;

    void law_walker_action(bool is_constable);
    void law_walker_chase();
    bool fight_enemy();
    figure_id closest_enemy(int max_distance) const;
    tile2i home_road() const;
    void arrest_tomb_robber_remake();
};
ANK_CONFIG_STRUCT(figure_constable::static_params, home_road_radius, crime_reduction, enemy_search_ticks,
  enemy_search_distance, native_distance_factor, enemy_distance_factor, animal_distance_factor, chased_animals,
  tomb_robber_arrest_distance)

#pragma once

#include "building/building.h"

class building_police_station : public building_impl {
public:
    BUILDING_METAINFO(BUILDING_POLICE_STATION, building_police_station, building_impl)

    using static_params = building_impl::static_params;
    BUILDING_STATIC_DATA_T

    virtual void spawn_figure() override;
    virtual e_overlay get_overlay() const override { return OVERLAY_CRIME; }
    virtual e_sound_channel_city sound_channel() const override { return SOUND_CHANNEL_CITY_POLICE; }
    virtual int animation_speed(int speed) const override { return 3; }
};

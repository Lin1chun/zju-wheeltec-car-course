#ifndef OBSTACLE_SAFETY_H
#define OBSTACLE_SAFETY_H

#include <stdint.h>

#define OBSTACLE_SECTOR_COUNT       5u
#define OBSTACLE_CENTER_SECTOR      2u
#define OBSTACLE_FRONT_FIRST_SECTOR 1u
#define OBSTACLE_FRONT_LAST_SECTOR  3u

#define OBSTACLE_STOP_MM            500u
#define OBSTACLE_CLEAR_MM           550u
#define OBSTACLE_CLEAR_TICKS        40u
#define OBSTACLE_MIN_VALID_MM       20u
#define OBSTACLE_MAX_VALID_MM       4000u
#define OBSTACLE_DISTANCE_INVALID   0xFFFFu

#define OBSTACLE_SCAN_SLOT_COUNT    8u
#define OBSTACLE_SCAN_DWELL_TICKS   16u
#define OBSTACLE_SCAN_FIRST_PING_TICK  9u
#define OBSTACLE_SCAN_SECOND_PING_TICK 15u

typedef struct
{
    uint16_t raw_mm[OBSTACLE_SECTOR_COUNT];
    uint16_t filtered_mm[OBSTACLE_SECTOR_COUNT];
    uint8_t valid[OBSTACLE_SECTOR_COUNT];
    uint8_t obstacle;
    uint16_t clear_ticks;
    uint16_t front_raw_mm;
    uint16_t front_filtered_mm;
    uint16_t all_raw_mm;
} obstacle_guard_t;

static inline uint8_t obstacle_sector_is_front(uint8_t sector)
{
    return (sector >= OBSTACLE_FRONT_FIRST_SECTOR &&
            sector <= OBSTACLE_FRONT_LAST_SECTOR);
}

static inline uint16_t obstacle_min_u16(uint16_t lhs, uint16_t rhs)
{
    return lhs < rhs ? lhs : rhs;
}

static inline void obstacle_guard_recompute(obstacle_guard_t *guard)
{
    uint8_t sector;

    guard->front_raw_mm = OBSTACLE_DISTANCE_INVALID;
    guard->front_filtered_mm = OBSTACLE_DISTANCE_INVALID;
    guard->all_raw_mm = OBSTACLE_DISTANCE_INVALID;

    for (sector = 0; sector < OBSTACLE_SECTOR_COUNT; ++sector) {
        if (!guard->valid[sector]) {
            continue;
        }

        guard->all_raw_mm =
            obstacle_min_u16(guard->all_raw_mm, guard->raw_mm[sector]);

        if (obstacle_sector_is_front(sector)) {
            guard->front_raw_mm =
                obstacle_min_u16(guard->front_raw_mm, guard->raw_mm[sector]);
            guard->front_filtered_mm = obstacle_min_u16(
                guard->front_filtered_mm, guard->filtered_mm[sector]);
        }
    }

    if (guard->front_raw_mm == OBSTACLE_DISTANCE_INVALID ||
        guard->front_raw_mm < OBSTACLE_STOP_MM) {
        guard->obstacle = 1;
        guard->clear_ticks = 0;
    }
}

static inline void obstacle_guard_init(obstacle_guard_t *guard)
{
    uint8_t sector;

    for (sector = 0; sector < OBSTACLE_SECTOR_COUNT; ++sector) {
        guard->raw_mm[sector] = OBSTACLE_DISTANCE_INVALID;
        guard->filtered_mm[sector] = 0;
        guard->valid[sector] = 0;
    }

    guard->obstacle = 1;
    guard->clear_ticks = 0;
    guard->front_raw_mm = OBSTACLE_DISTANCE_INVALID;
    guard->front_filtered_mm = OBSTACLE_DISTANCE_INVALID;
    guard->all_raw_mm = OBSTACLE_DISTANCE_INVALID;
}

static inline void obstacle_guard_add_sample(obstacle_guard_t *guard,
                                              uint8_t sector,
                                              uint16_t mm)
{
    if (sector >= OBSTACLE_SECTOR_COUNT ||
        mm < OBSTACLE_MIN_VALID_MM ||
        mm > OBSTACLE_MAX_VALID_MM) {
        return;
    }

    guard->raw_mm[sector] = mm;

    if (!guard->valid[sector] ||
        mm <= guard->filtered_mm[sector]) {
        guard->filtered_mm[sector] = mm;
    } else {
        guard->filtered_mm[sector] =
            (uint16_t)(((uint32_t)guard->filtered_mm[sector] * 3u + mm) / 4u);
    }

    guard->valid[sector] = 1;
    obstacle_guard_recompute(guard);
}

static inline void obstacle_guard_invalidate(obstacle_guard_t *guard,
                                              uint8_t sector)
{
    if (sector >= OBSTACLE_SECTOR_COUNT) {
        return;
    }

    guard->valid[sector] = 0;
    guard->raw_mm[sector] = OBSTACLE_DISTANCE_INVALID;
    guard->filtered_mm[sector] = 0;
    obstacle_guard_recompute(guard);
}

static inline void obstacle_guard_tick_10ms(obstacle_guard_t *guard,
                                             uint16_t ticks)
{
    while (ticks-- != 0u) {
        if (guard->front_raw_mm == OBSTACLE_DISTANCE_INVALID ||
            guard->front_raw_mm < OBSTACLE_STOP_MM) {
            guard->obstacle = 1;
            guard->clear_ticks = 0;
            continue;
        }

        if (guard->front_raw_mm > OBSTACLE_CLEAR_MM) {
            if (guard->obstacle) {
                if (guard->clear_ticks < OBSTACLE_CLEAR_TICKS) {
                    ++guard->clear_ticks;
                }
                if (guard->clear_ticks >= OBSTACLE_CLEAR_TICKS) {
                    guard->obstacle = 0;
                }
            }
        } else {
            /* Keep the previous state inside the 500/550 mm band. */
            guard->clear_ticks = 0;
        }
    }
}

static inline uint8_t obstacle_scan_sector_for_slot(uint8_t slot)
{
    static const uint8_t order[OBSTACLE_SCAN_SLOT_COUNT] = {
        OBSTACLE_CENTER_SECTOR,
        1u,
        OBSTACLE_CENTER_SECTOR,
        3u,
        OBSTACLE_CENTER_SECTOR,
        0u,
        OBSTACLE_CENTER_SECTOR,
        4u
    };

    return order[slot % OBSTACLE_SCAN_SLOT_COUNT];
}

static inline uint8_t obstacle_tx_rising_edge(uint8_t obstacle,
                                               uint8_t previous_obstacle)
{
    return (uint8_t)(obstacle && !previous_obstacle);
}

#endif

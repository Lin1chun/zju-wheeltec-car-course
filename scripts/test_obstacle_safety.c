#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "obstacle_safety.h"

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);\
            return EXIT_FAILURE;                                           \
        }                                                                  \
    } while (0)

static void add_all(obstacle_guard_t *guard, uint16_t mm)
{
    uint8_t sector;

    for (sector = 0; sector < OBSTACLE_SECTOR_COUNT; ++sector) {
        obstacle_guard_add_sample(guard, sector, mm);
    }
}

static int test_close_sample_latches_immediately(void)
{
    obstacle_guard_t guard;

    obstacle_guard_init(&guard);
    add_all(&guard, 4000);
    obstacle_guard_tick_10ms(&guard, 40);
    CHECK(guard.obstacle == 0);

    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 300);

    CHECK(guard.obstacle == 1);
    CHECK(guard.front_raw_mm == 300);
    return EXIT_SUCCESS;
}

static int test_clear_uses_hysteresis_and_delay(void)
{
    obstacle_guard_t guard;

    obstacle_guard_init(&guard);
    add_all(&guard, 4000);
    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 400);
    CHECK(guard.obstacle == 1);

    add_all(&guard, 520);
    obstacle_guard_tick_10ms(&guard, 100);
    CHECK(guard.obstacle == 1);

    add_all(&guard, 551);
    obstacle_guard_tick_10ms(&guard, OBSTACLE_CLEAR_TICKS - 1);
    CHECK(guard.obstacle == 1);

    obstacle_guard_tick_10ms(&guard, 1);
    CHECK(guard.obstacle == 0);
    return EXIT_SUCCESS;
}

static int test_missing_front_echo_fails_safe(void)
{
    obstacle_guard_t guard;
    uint8_t sector;

    obstacle_guard_init(&guard);
    add_all(&guard, 4000);
    obstacle_guard_tick_10ms(&guard, 40);
    CHECK(guard.obstacle == 0);

    for (sector = OBSTACLE_FRONT_FIRST_SECTOR;
         sector <= OBSTACLE_FRONT_LAST_SECTOR;
         ++sector) {
        obstacle_guard_invalidate(&guard, sector);
    }
    obstacle_guard_tick_10ms(&guard, 1);

    CHECK(guard.obstacle == 1);
    CHECK(guard.front_raw_mm == OBSTACLE_DISTANCE_INVALID);
    return EXIT_SUCCESS;
}

static int test_filter_attacks_fast_and_releases_slowly(void)
{
    obstacle_guard_t guard;

    obstacle_guard_init(&guard);
    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 4000);
    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 300);
    CHECK(guard.filtered_mm[OBSTACLE_CENTER_SECTOR] == 300);

    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 1000);
    CHECK(guard.filtered_mm[OBSTACLE_CENTER_SECTOR] == 475);
    return EXIT_SUCCESS;
}

static int test_schedule_prioritizes_center_and_visits_every_sector(void)
{
    uint8_t seen_mask = 0;
    uint8_t slot;

    for (slot = 0; slot < OBSTACLE_SCAN_SLOT_COUNT; ++slot) {
        uint8_t sector = obstacle_scan_sector_for_slot(slot);
        CHECK(sector < OBSTACLE_SECTOR_COUNT);
        seen_mask |= (uint8_t)(1u << sector);
        if ((slot & 1u) == 0) {
            CHECK(sector == OBSTACLE_CENTER_SECTOR);
        }
    }

    CHECK(seen_mask == 0x1Fu);
    return EXIT_SUCCESS;
}

static int test_sample_range_boundaries(void)
{
    obstacle_guard_t guard;

    obstacle_guard_init(&guard);
    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 19);
    CHECK(guard.valid[OBSTACLE_CENTER_SECTOR] == 0);
    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 20);
    CHECK(guard.valid[OBSTACLE_CENTER_SECTOR] == 1);
    obstacle_guard_add_sample(&guard, OBSTACLE_CENTER_SECTOR, 4001);
    CHECK(guard.raw_mm[OBSTACLE_CENTER_SECTOR] == 20);
    return EXIT_SUCCESS;
}

static int test_obstacle_event_only_on_rising_edge(void)
{
    CHECK(obstacle_tx_rising_edge(0, 0) == 0);
    CHECK(obstacle_tx_rising_edge(1, 0) == 1);
    CHECK(obstacle_tx_rising_edge(1, 1) == 0);
    CHECK(obstacle_tx_rising_edge(0, 1) == 0);
    return EXIT_SUCCESS;
}

static int test_scan_waits_for_servo_to_settle(void)
{
    uint16_t previous_center_tick = 0;
    uint16_t max_gap_ticks = 0;
    uint8_t cycle;
    uint8_t slot;

    CHECK(OBSTACLE_SCAN_DWELL_TICKS >= 16);
    CHECK(OBSTACLE_SCAN_FIRST_PING_TICK >= 9);
    CHECK(OBSTACLE_SCAN_SECOND_PING_TICK > OBSTACLE_SCAN_FIRST_PING_TICK);
    CHECK(OBSTACLE_SCAN_SECOND_PING_TICK < OBSTACLE_SCAN_DWELL_TICKS);

    for (cycle = 0; cycle < 2; ++cycle) {
        for (slot = 0; slot < OBSTACLE_SCAN_SLOT_COUNT; ++slot) {
            uint16_t base;
            uint16_t first;
            uint16_t second;

            if (obstacle_scan_sector_for_slot(slot) != OBSTACLE_CENTER_SECTOR) {
                continue;
            }

            base = (uint16_t)((cycle * OBSTACLE_SCAN_SLOT_COUNT + slot) *
                              OBSTACLE_SCAN_DWELL_TICKS);
            first = (uint16_t)(base + OBSTACLE_SCAN_FIRST_PING_TICK);
            second = (uint16_t)(base + OBSTACLE_SCAN_SECOND_PING_TICK);

            if (previous_center_tick != 0 &&
                (uint16_t)(first - previous_center_tick) > max_gap_ticks) {
                max_gap_ticks = (uint16_t)(first - previous_center_tick);
            }
            previous_center_tick = second;
        }
    }

    CHECK(max_gap_ticks <= 26);
    return EXIT_SUCCESS;
}

int main(void)
{
    CHECK(test_close_sample_latches_immediately() == EXIT_SUCCESS);
    CHECK(test_clear_uses_hysteresis_and_delay() == EXIT_SUCCESS);
    CHECK(test_missing_front_echo_fails_safe() == EXIT_SUCCESS);
    CHECK(test_filter_attacks_fast_and_releases_slowly() == EXIT_SUCCESS);
    CHECK(test_schedule_prioritizes_center_and_visits_every_sector() == EXIT_SUCCESS);
    CHECK(test_sample_range_boundaries() == EXIT_SUCCESS);
    CHECK(test_obstacle_event_only_on_rising_edge() == EXIT_SUCCESS);
    CHECK(test_scan_waits_for_servo_to_settle() == EXIT_SUCCESS);

    puts("obstacle safety tests: PASS");
    return EXIT_SUCCESS;
}

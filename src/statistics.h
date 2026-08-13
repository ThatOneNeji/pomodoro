/**
 * @file statistics.h
 * @brief Lifetime usage counters (cycles completed, time worked, time on break), persisted via Preferences.
 */
#ifndef STATISTICS_H
#define STATISTICS_H

#include <Arduino.h>
#include <Preferences.h>

/// Increment the persisted count of completed pomodoro cycles by one.
void incrementTotalCycles();

/**
 * @brief Add to the persisted total working time.
 * @param ms Duration to add, in milliseconds (stored as whole minutes).
 */
void incrementTotalTime(unsigned long ms);

/**
 * @brief Add to the persisted total break time.
 * @param ms Duration to add, in milliseconds (stored as whole minutes).
 */
void incrementTotalBreakTime(unsigned long ms);

/**
 * @brief Read the persisted lifetime statistics.
 * @param[out] totalCycles Total completed cycles.
 * @param[out] totalTime Total working time, in minutes.
 * @param[out] totalBreakTime Total break time, in minutes.
 */
void getStatistics(unsigned int *totalCycles, unsigned long *totalTime, unsigned long *totalBreakTime);

/// Reset all persisted statistics (cycles, time, break time) back to zero.
void resetStatistics();

#endif
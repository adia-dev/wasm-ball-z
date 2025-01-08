#pragma once
#include "math/vector2.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

namespace wbz {
namespace ai {

struct State {
  // TODO: last_hit state ???
  // TODO: ADD LAST 10 actions of the opponents encoded !!!!! bytemask ?
  // Triple level radar, close, medium, far
  int distance_bin;        // Distance to opponent (discretized)
  int relative_x_bin;      // X position relative to opponent (discretized)
  int relative_y_bin;      // Y position relative to opponent (discretized)
  bool opponent_attacking; // Is opponent currently attacking?
  bool low_health;         // Is our health below 30%?
  bool opponent_in_radar;  // Is opponent currently in range?

  std::string to_string() const {
    return std::to_string(distance_bin) + "," + std::to_string(relative_x_bin) +
           "," + std::to_string(relative_y_bin) + "," +
           std::to_string(opponent_attacking) + "," +
           std::to_string(low_health) + "," + std::to_string(opponent_in_radar);
  }
};

// 240 000
enum class Action {
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  LIGHT_PUNCH,
  HEAVY_PUNCH,
  LIGHT_KICK,
  HEAVY_KICK,
  BLOCK,
  IDLE,
  ACTION_COUNT
};

class QLearningAgent {
public:
  QLearningAgent(float learning_rate = 0.1f, float discount_factor = 0.95f,
                 float exploration_rate = 1.0f);

  State get_state(const Vector2f &agent_pos, const Vector2f &opponent_pos,
                  bool is_opponent_attacking, float health_percent);

  Action select_action(const State &state);

  void update(const State &state, Action action, float reward,
              const State &next_state);

  float calculate_reward(float health_change, float opponent_health_change,
                         float distance, bool hit_landed, bool got_hit,
                         float time_since_last_action, bool radar_in_range);

  void decay_exploration();

  float get_exploration_rate() const;

  void log_action_selection(const State &state, Action action, float q_value);

private:
  std::string action_to_string(Action action) const;

  std::string state_to_string(const State &state) const;

private:
  float learning_rate;
  float discount_factor;
  float exploration_rate;
  std::mt19937 rng;

  int _recent_hits = 0;                 // For tracking combo multiplier
  float _last_known_distance = 0.0f;    // For tracking distance changes
  std::vector<float> _distance_history; // For calculating moving average

  // New helper method to track distance trends
  void update_distance_history(float current_distance);

  float get_distance_trend();

  // Q-table: state string -> array of Q-values for each action
  std::unordered_map<std::string, std::vector<float>> q_table;

  int discretize_distance(float distance);

  int discretize_position(float pos);

  float get_q_value(const std::string &state_key, Action action);

  float get_max_q_value(const std::string &state_key);

  Action get_best_action(const State &state);
};

} // namespace ai
} // namespace wbz

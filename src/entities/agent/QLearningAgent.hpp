#pragma once
#include "math/vector2.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>

namespace wbz {
namespace ai {

// We'll discretize our state space to make it manageable
struct State {
  int distance_bin;        // Distance to opponent (discretized)
  int relative_x_bin;      // X position relative to opponent (discretized)
  int relative_y_bin;      // Y position relative to opponent (discretized)
  bool opponent_attacking; // Is opponent currently attacking?
  bool low_health;         // Is our health below 30%?

  // Create a string key for our state to use in the Q-table
  std::string to_string() const {
    return std::to_string(distance_bin) + "," + std::to_string(relative_x_bin) +
           "," + std::to_string(relative_y_bin) + "," +
           std::to_string(opponent_attacking) + "," +
           std::to_string(low_health);
  }
};

// Define possible actions the agent can take
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
                 float exploration_rate = 1.0f)
      : learning_rate(learning_rate), discount_factor(discount_factor),
        exploration_rate(exploration_rate), rng(std::random_device{}()) {}

  State get_state(const Vector2f &agent_pos, const Vector2f &opponent_pos,
                  bool is_opponent_attacking, float health_percent) {
    Vector2f relative_pos = opponent_pos.sub(agent_pos);
    float distance = relative_pos.mag();

    // Discretize continuous values into bins
    State state;
    state.distance_bin = discretize_distance(distance);
    state.relative_x_bin = discretize_position(relative_pos.x);
    state.relative_y_bin = discretize_position(relative_pos.y);
    state.opponent_attacking = is_opponent_attacking;
    state.low_health = health_percent < 0.3f;

    return state;
  }

  Action select_action(const State &state) {
    // Exploration: Random action
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(rng) < exploration_rate) {
      std::uniform_int_distribution<int> action_dist(
          0, static_cast<int>(Action::ACTION_COUNT) - 1);
      return static_cast<Action>(action_dist(rng));
    }

    // Exploitation: Choose best action based on Q-values
    return get_best_action(state);
  }

  void update(const State &state, Action action, float reward,
              const State &next_state) {
    std::string state_key = state.to_string();
    std::string next_state_key = next_state.to_string();

    // Get current Q value
    float current_q = get_q_value(state_key, action);

    // Get maximum Q value for next state
    float max_next_q = get_max_q_value(next_state_key);

    // Q-learning update formula
    float new_q =
        current_q +
        learning_rate * (reward + discount_factor * max_next_q - current_q);

    // Update Q-table
    q_table[state_key][static_cast<int>(action)] = new_q;
  }

  float calculate_reward(float health_change, float opponent_health_change,
                         float distance, bool hit_landed, bool got_hit,
                         float time_since_last_action) {
    float reward = 0.0f;

    // Reward for dealing damage with logging
    if (opponent_health_change > 0) {
      reward += opponent_health_change * 10.0f;
      std::cout << "Reward for dealing " << opponent_health_change
                << " damage: +" << (opponent_health_change * 10.0f)
                << std::endl;
    }

    // Penalty for taking damage
    if (health_change > 0) {
      reward -= health_change * 8.0f;
      std::cout << "Penalty for taking " << health_change
                << " damage: " << -(health_change * 8.0f) << std::endl;
    }

    // Distance management rewards
    if (distance > 100.0f && distance < 200.0f) {
      reward += 0.2f;
      std::cout << "Reward for optimal distance (" << distance << "): +0.2"
                << std::endl;
    } else if (distance < 50.0f) {
      reward -= 0.3f; // Penalty for being too close
      std::cout << "Penalty for being too close (" << distance << "): -0.3"
                << std::endl;
    }

    // Combat success rewards
    if (hit_landed) {
      reward += 5.0f;
      std::cout << "Reward for landing a hit: +5.0" << std::endl;
    }
    if (got_hit) {
      reward -= 3.0f;
      std::cout << "Penalty for getting hit: -3.0" << std::endl;
    }

    // Inactivity penalty
    if (time_since_last_action > 2.0f) { // If no action for 2 seconds
      float inactivity_penalty = -0.5f * (time_since_last_action - 2.0f);
      reward += inactivity_penalty;
      std::cout << "Inactivity penalty (" << time_since_last_action
                << "s): " << inactivity_penalty << std::endl;
    }

    std::cout << "Total reward: " << reward << std::endl;
    return reward;
  }

  void decay_exploration() {
    // Gradually reduce exploration rate
    exploration_rate = std::max(0.01f, exploration_rate * 0.995f);
  }

private:
  float learning_rate;
  float discount_factor;
  float exploration_rate;
  std::mt19937 rng;

  // Q-table: state string -> array of Q-values for each action
  std::unordered_map<std::string, std::vector<float>> q_table;

  // Helper methods for state discretization
  int discretize_distance(float distance) {
    // Convert continuous distance into discrete bins
    if (distance < 50.0f)
      return 0; // Very close
    if (distance < 100.0f)
      return 1; // Close
    if (distance < 200.0f)
      return 2; // Medium
    if (distance < 300.0f)
      return 3; // Far
    return 4;   // Very far
  }

  int discretize_position(float pos) {
    // Convert continuous position into discrete bins
    if (pos < -200.0f)
      return 0; // Far negative
    if (pos < -100.0f)
      return 1; // Medium negative
    if (pos < 100.0f)
      return 2; // Near zero
    if (pos < 200.0f)
      return 3; // Medium positive
    return 4;   // Far positive
  }

  float get_q_value(const std::string &state_key, Action action) {
    // Initialize Q-values if state is new
    if (q_table.find(state_key) == q_table.end()) {
      q_table[state_key] =
          std::vector<float>(static_cast<int>(Action::ACTION_COUNT), 0.0f);
    }
    return q_table[state_key][static_cast<int>(action)];
  }

  float get_max_q_value(const std::string &state_key) {
    if (q_table.find(state_key) == q_table.end()) {
      return 0.0f;
    }

    const auto &q_values = q_table[state_key];
    return *std::max_element(q_values.begin(), q_values.end());
  }

  Action get_best_action(const State &state) {
    std::string state_key = state.to_string();

    // Initialize Q-values if needed
    if (q_table.find(state_key) == q_table.end()) {
      return static_cast<Action>(std::uniform_int_distribution<int>(
          0, static_cast<int>(Action::ACTION_COUNT) - 1)(rng));
    }

    // Find action with maximum Q-value
    const auto &q_values = q_table[state_key];
    int best_action =
        std::max_element(q_values.begin(), q_values.end()) - q_values.begin();

    return static_cast<Action>(best_action);
  }
};

} // namespace ai
} // namespace wbz

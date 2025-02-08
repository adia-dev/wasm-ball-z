#include "QLearningAgent.hpp"

namespace wbz {
namespace ai {
QLearningAgent::QLearningAgent(float learning_rate, float discount_factor,
                               float exploration_rate)
    : learning_rate(learning_rate), discount_factor(discount_factor),
      exploration_rate(exploration_rate), rng(std::random_device{}()) {

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        State state;
        state.distance_bin = i;
        state.relative_x_bin = j;
        state.relative_y_bin = k;
        state.opponent_attacking = false;
        state.low_health = false;
        state.opponent_in_radar = true;

        std::string state_key = state.to_string();
        q_table[state_key] =
            std::vector<float>(static_cast<int>(Action::ACTION_COUNT), 0.1f);
      }
    }
  }
}
State QLearningAgent::get_state(const Vector2f &agent_pos,
                                const Vector2f &opponent_pos,
                                bool is_opponent_attacking,
                                float health_percent) {
  Vector2f relative_pos = opponent_pos.sub(agent_pos);
  float distance = relative_pos.mag();

  State state;
  state.distance_bin = discretize_distance(distance);
  state.relative_x_bin = discretize_position(relative_pos.x);
  state.relative_y_bin = discretize_position(relative_pos.y);
  state.opponent_attacking = is_opponent_attacking;
  state.low_health = health_percent < 0.3f;

  return state;
}

Action QLearningAgent::select_action(const State &state) {
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  if (!state.opponent_in_radar) {

    if (dist(rng) < 0.7f) {

      std::vector<Action> search_actions = {Action::MOVE_LEFT,
                                            Action::MOVE_RIGHT, Action::MOVE_UP,
                                            Action::MOVE_DOWN};
      return search_actions[std::uniform_int_distribution<int>(0, 3)(rng)];
    }
  }

  if (state.distance_bin <= 2) {
    if (dist(rng) < 0.4f) {
      std::vector<Action> attack_actions = {
          Action::LIGHT_PUNCH, Action::HEAVY_PUNCH, Action::LIGHT_KICK,
          Action::HEAVY_KICK};
      return attack_actions[std::uniform_int_distribution<int>(0, 3)(rng)];
    }
  }

  if (dist(rng) < exploration_rate) {
    return static_cast<Action>(std::uniform_int_distribution<int>(
        0, static_cast<int>(Action::ACTION_COUNT) - 1)(rng));
  }

  return get_best_action(state);
}

void QLearningAgent::update(const State &state, Action action, float reward,
                            const State &next_state) {
  std::string state_key = state.to_string();
  std::string next_state_key = next_state.to_string();

  float current_q = get_q_value(state_key, action);

  float max_next_q = get_max_q_value(next_state_key);

  float new_q =
      current_q +
      learning_rate * (reward + discount_factor * max_next_q - current_q);

  q_table[state_key][static_cast<int>(action)] = new_q;
}

float QLearningAgent::calculate_reward(float health_change,
                                       float opponent_health_change,
                                       float distance, bool hit_landed,
                                       bool got_hit,
                                       float time_since_last_action,
                                       bool radar_in_range) {
  float reward = 0.0f;

  const float OPTIMAL_COMBAT_DISTANCE = 120.0f;
  const float CLOSE_RANGE = 80.0f;
  const float MID_RANGE = 200.0f;
  const float FAR_RANGE = 300.0f;

  float distance_reward = 0.0f;
  if (radar_in_range) {

    float distance_deviation =
        std::abs(distance - OPTIMAL_COMBAT_DISTANCE) / OPTIMAL_COMBAT_DISTANCE;

    distance_reward = 2.0f * (1.0f - std::min(1.0f, distance_deviation));

    if (distance < CLOSE_RANGE) {
      distance_reward -= 1.0f;
      std::cout << "⚠️ Too close for comfort! Distance penalty: -1.0"
                << std::endl;
    } else if (distance > FAR_RANGE) {
      distance_reward -= 2.0f;
      std::cout << "⚠️ Too far to be effective! Distance penalty: -2.0"
                << std::endl;
    } else if (std::abs(distance - OPTIMAL_COMBAT_DISTANCE) < 20.0f) {
      distance_reward += 1.0f;
      std::cout << "✨ Perfect combat range! Bonus: +1.0" << std::endl;
    }

    std::cout << "📏 Distance reward/penalty: " << distance_reward << std::endl;
  } else {

    distance_reward = -3.0f;
    std::cout << "🔍 Out of radar range penalty: -3.0" << std::endl;
  }
  reward += distance_reward;

  static float previous_distance_deviation = std::numeric_limits<float>::max();
  float current_distance_deviation =
      std::abs(distance - OPTIMAL_COMBAT_DISTANCE);

  if (previous_distance_deviation != std::numeric_limits<float>::max()) {
    float improvement =
        previous_distance_deviation - current_distance_deviation;
    if (improvement > 0) {
      float improvement_reward = improvement * 0.5f;
      reward += improvement_reward;
      std::cout << "⬆️ Moving toward optimal range: +" << improvement_reward
                << std::endl;
    }
  }
  previous_distance_deviation = current_distance_deviation;

  if (hit_landed) {

    float hit_reward = 5.0f;

    if (std::abs(distance - OPTIMAL_COMBAT_DISTANCE) < 30.0f) {
      hit_reward *= 1.5f;
      std::cout << "🎯 Perfect range hit bonus! Reward multiplier: 1.5x"
                << std::endl;
    }

    reward += hit_reward;
    std::cout << "💥 Hit landed reward: +" << hit_reward << std::endl;
  }

  if (got_hit) {
    float defense_penalty = -4.0f;
    if (distance < CLOSE_RANGE) {
      defense_penalty *= 1.5f;
      std::cout << "💔 Vulnerable position hit! Extra penalty applied"
                << std::endl;
    }
    reward += defense_penalty;
    std::cout << "💔 Got hit penalty: " << defense_penalty << std::endl;
  }

  if (time_since_last_action > 0.5f) {
    float inactivity_penalty = -0.3f * time_since_last_action;
    if (!radar_in_range || distance > FAR_RANGE) {
      inactivity_penalty *= 2.0f;
      std::cout << "⏰ Double inactivity penalty due to poor positioning"
                << std::endl;
    }
    reward += inactivity_penalty;
    std::cout << "⏰ Inactivity penalty: " << inactivity_penalty << std::endl;
  }

  std::cout << "📊 Final reward calculation: " << reward << std::endl;
  return reward;
}

void QLearningAgent::decay_exploration() {

  exploration_rate = std::max(0.01f, exploration_rate * 0.995f);
}
float QLearningAgent::get_exploration_rate() const { return exploration_rate; }
void QLearningAgent::log_action_selection(const State &state, Action action,
                                          float q_value) {
  std::cout << "┌─ Action Selection ─────────────────\n";
  std::cout << "│ Selected: " << action_to_string(action) << "\n";
  std::cout << "│ Q-Value: " << q_value << "\n";
  std::cout << "│ State: " << state_to_string(state) << "\n";
  std::cout << "└────────────────────────────────────\n";
}
std::string QLearningAgent::action_to_string(Action action) const {
  switch (action) {
  case Action::MOVE_LEFT:
    return "MOVE_LEFT";
  case Action::MOVE_RIGHT:
    return "MOVE_RIGHT";
  case Action::MOVE_UP:
    return "MOVE_UP";
  case Action::MOVE_DOWN:
    return "MOVE_DOWN";
  case Action::LIGHT_PUNCH:
    return "LIGHT_PUNCH";
  case Action::HEAVY_PUNCH:
    return "HEAVY_PUNCH";
  case Action::LIGHT_KICK:
    return "LIGHT_KICK";
  case Action::HEAVY_KICK:
    return "HEAVY_KICK";
  case Action::BLOCK:
    return "BLOCK";
  case Action::IDLE:
    return "IDLE";
  default:
    return "UNKNOWN";
  }
}
std::string QLearningAgent::state_to_string(const State &state) const {
  std::stringstream ss;
  ss << "Distance: " << state.distance_bin << ", RelX: " << state.relative_x_bin
     << ", RelY: " << state.relative_y_bin
     << ", OppAtk: " << (state.opponent_attacking ? "Yes" : "No")
     << ", LowHP: " << (state.low_health ? "Yes" : "No")
     << ", InRadar: " << (state.opponent_in_radar ? "Yes" : "No");
  return ss.str();
}
int QLearningAgent::discretize_distance(float distance) {

  if (distance < 50.0f)
    return 0;
  if (distance < 100.0f)
    return 1;
  if (distance < 200.0f)
    return 2;
  if (distance < 300.0f)
    return 3;
  return 4;
}
int QLearningAgent::discretize_position(float pos) {

  if (pos < -200.0f)
    return 0;
  if (pos < -100.0f)
    return 1;
  if (pos < 100.0f)
    return 2;
  if (pos < 200.0f)
    return 3;
  return 4;
}
float QLearningAgent::get_q_value(const std::string &state_key, Action action) {

  if (q_table.find(state_key) == q_table.end()) {
    q_table[state_key] =
        std::vector<float>(static_cast<int>(Action::ACTION_COUNT), 0.0f);
  }
  return q_table[state_key][static_cast<int>(action)];
}
float QLearningAgent::get_max_q_value(const std::string &state_key) {
  if (q_table.find(state_key) == q_table.end()) {
    return 0.0f;
  }

  const auto &q_values = q_table[state_key];
  return *std::max_element(q_values.begin(), q_values.end());
}
Action QLearningAgent::get_best_action(const State &state) {
  std::string state_key = state.to_string();

  if (q_table.find(state_key) == q_table.end()) {
    return static_cast<Action>(std::uniform_int_distribution<int>(
        0, static_cast<int>(Action::ACTION_COUNT) - 1)(rng));
  }

  const auto &q_values = q_table[state_key];
  int best_action =
      std::max_element(q_values.begin(), q_values.end()) - q_values.begin();

  return static_cast<Action>(best_action);
}
float QLearningAgent::get_distance_trend() {
  if (_distance_history.size() < 2)
    return 0.0f;
  return _distance_history.back() - _distance_history.front();
}
void QLearningAgent::update_distance_history(float current_distance) {
  _distance_history.push_back(current_distance);
  if (_distance_history.size() > 60) {
    _distance_history.erase(_distance_history.begin());
  }
}
} // namespace ai
} // namespace wbz

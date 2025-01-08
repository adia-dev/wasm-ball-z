#pragma once
#include "character.hpp"
#include "entities/agent/QLearningAgent.hpp"
#include <memory>

namespace wbz {
namespace entities {

class AICharacter : public Character {
public:
  AICharacter(const Sprite &sprite, const CombatStats &stats = CombatStats())
      : Character(sprite, stats),
        ai_agent(std::make_unique<ai::QLearningAgent>()),
        _previous_health(stats.max_health), _previous_opponent_health(0),
        _hit_landed(false), _got_hit(false), _time_since_last_action(0.0f),
        _training_episode(0),

        _radar_radius(150.0f), _max_radar_radius(500.0f),
        _radar_expand_speed(20.0f) {
    std::cout << "\nInitializing AI Character with stats:"
              << "\nHealth: " << stats.max_health
              << "\nStamina: " << stats.max_stamina
              << "\nMovement Speed: " << stats.movement_speed
              << "\nBase Defense: " << stats.base_defense << std::endl;
  }

  void update(double delta_time) override;
  void render(SDL_Renderer *renderer) const override;

  void start_new_episode();

  void set_opponent(Character *opponent);

  void on_hit_landed();
  void on_got_hit();

protected:
  void handle_defeat() override;

private:
  void log_episode_start();

  void log_episode_end();

  void log_combat_event(const std::string &event_type,
                        const std::string &details);

private:
  std::unique_ptr<ai::QLearningAgent> ai_agent;
  Character *_opponent = nullptr;

  ai::State _previous_state;
  ai::Action _previous_action;
  bool _has_previous_state = false;

  float _previous_health;
  float _previous_opponent_health;
  float _episode_timer;
  float _time_since_last_action;
  int _training_episode;
  int _hits_landed;
  float _average_distance;
  int _hits_taken;
  bool _hit_landed;
  bool _got_hit;

  float _radar_radius;
  float _max_radar_radius;
  float _radar_expand_speed;

  void execute_action(ai::Action action);

  bool is_opponent_in_radar() const;

  void draw_radar(SDL_Renderer *renderer) const;
};

} // namespace entities
} // namespace wbz

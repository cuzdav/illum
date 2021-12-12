#include "AnalysisBoard.hpp"
#include "BasicWallLayout.hpp"
#include "BoardModel.hpp"
#include "Solver.hpp"
#include "olcPixelGameEngine.h"
#include "olcRetroMenu.hpp"
#include "utils/EnumUtils.hpp"
#include <memory>
#include <random>

class Illum : public olc::PixelGameEngine {
public:
  Illum();

  static constexpr int   RENDER_FPS      = 30;
  static constexpr float RENDER_INTERVAL = 1.0 / RENDER_FPS;
  static constexpr int   BOARD_SIZES[]   = {6, 8, 10, 15, 20};

public:
  bool OnUserCreate() override;
  bool OnUserUpdate(float fElapsedTime) override;

  bool render();

  // when the model updates, it calls back into this
  void on_state_change(model::Action    action,
                       model::CellState prev_state,
                       model::CellState to_state,
                       model::Coord     coord);

private:
  model::Coord get_mouse_pos_as_tile_coord() const;

  bool create_menu();
  bool update_menu();
  bool start_game();
  bool update_game();

  bool render_menu();
  bool render_game();
  void play_tile_at(model::CellState play_tile);

private:
  class StateChange : public model::StateChangeHandler {
  public:
    StateChange(Illum & owner) : owner_(owner) {}
    void on_state_change(model::Action    action,
                         model::CellState prev_state,
                         model::CellState to_state,
                         model::Coord     coord) override;

  private:
    Illum & owner_;
  };

  enum class State { Menu, StartGame, Playing, Exit };
  enum class MenuId {
    Play,
    Tutorial,
    Settings,

    // Difficulty
    VeryEasy,
    Easy,
    Intermediate,
    Hard,
    Expert,

    // Size
    MinVerySmall,
    MinSmall,
    MinMedium,
    MinLarge,
    MinHuge,
    MaxVerySmall,
    MaxSmall,
    MaxMedium,
    MaxLarge,
    MaxHuge,

    // Meta option
    Back,

  };

  enum class Difficulty { VeryEasy, Easy, Intermediate, Hard, Expert };
  friend char const * to_string(Difficulty);

  std::unique_ptr<olc::Sprite> menu_sprite_;
  olc::popup::Menu             menu;
  olc::popup::Manager          menu_manager;
  State                        state_ = State::Menu;

  Difficulty difficulty_         = Difficulty::VeryEasy;
  int        min_board_size_idx_ = 0;
  int        max_board_size_idx_ = 3;
  int        tile_width_         = 0;
  int        tile_height_        = 0;
  int        bulbs_in_solution_  = 0;
  int        bulbs_played_       = 0;

  model::BoardModel     model_;
  solver::PositionBoard position_;
  std::mt19937          twister_rng_;
};

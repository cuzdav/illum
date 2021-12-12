#include "Illum.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include <chrono>
#include <memory>
#include <random>

// Note: characters are fix-width "8 pixels", so "-4" in DrawStrings is to
// center a char.
static constexpr int HALF_CHAR_PXLS = 4;

static constexpr int ROW_PADDING_ABOVE = 2;
static constexpr int ROW_PADDING_BELOW = 1;
static constexpr int COL_PADDING       = 1;

char const *
to_string(Illum::Difficulty d) {
  using enum Illum::Difficulty;
  switch (d) {
    case VeryEasy:
      return "Very Easy";
    case Easy:
      return "Easy";
    case Intermediate:
      return "Intermediate";
    case Hard:
      return "Hard";
    case Expert:
      return "Expert";
    default:
      return "( ??? Unhandled Difficulty ??? )";
  }
}

void
Illum::StateChange::on_state_change(model::Action    action,
                                    model::CellState prev_state,
                                    model::CellState to_state,
                                    model::Coord     coord) {
  owner_.on_state_change(action, prev_state, to_state, coord);
}

Illum::Illum()
    : twister_rng_(
          std::chrono::system_clock::now().time_since_epoch().count()) {
  sAppName = "Illum";
}

bool
Illum::create_menu() {
  menu_sprite_ = std::make_unique<olc::Sprite>("./RetroMenu.png");

  menu["main"].SetTable(1, 3);
  menu["main"]["Play"].SetID(+MenuId::Play);
  menu["main"]["Tutorial"].SetID(+MenuId::Tutorial).Enable(false);
  auto & settings = menu["main"]["Settings"].SetTable(1, 4);

  auto & difficulty = settings["Difficulty"].SetTable(1, 5);
  difficulty["Very Easy"].SetID(+MenuId::VeryEasy);
  difficulty["Easy"].SetID(+MenuId::Easy);
  difficulty["Intermediate"].SetID(+MenuId::Intermediate);
  difficulty["Hard"].SetID(+MenuId::Hard);
  difficulty["Expert"].SetID(+MenuId::Expert);
  difficulty["(Back)"].SetID(+MenuId::Back);

  auto & min = settings["Min Board Size"].SetTable(1, 6);
  min["Very Small"].SetID(+MenuId::MinVerySmall);
  min["Small"].SetID(+MenuId::MinSmall);
  min["Medium"].SetID(+MenuId::MinMedium);
  min["Large"].SetID(+MenuId::MinLarge);
  min["Huge"].SetID(+MenuId::MinHuge);
  min["(Back)"].SetID(+MenuId::Back);

  auto & max = settings["Max Board Size"].SetTable(1, 6);
  max["Very Small"].SetID(+MenuId::MaxVerySmall);
  max["Small"].SetID(+MenuId::MaxSmall);
  max["Medium"].SetID(+MenuId::MaxMedium);
  max["Large"].SetID(+MenuId::MaxLarge);
  max["Huge"].SetID(+MenuId::MaxHuge);
  max["(Back)"].SetID(+MenuId::Back);

  settings["(Back)"].SetID(+MenuId::Back);

  menu.Build();
  menu_manager.Open(&menu["main"]);

  return true;
}

bool
Illum::update_menu() {
  olc::popup::Menu * command = nullptr;

  if (GetKey(olc::Key::UP).bPressed) {
    menu_manager.OnUp();
  }
  if (GetKey(olc::Key::DOWN).bPressed) {
    menu_manager.OnDown();
  }
  if (GetKey(olc::Key::LEFT).bPressed) {
    menu_manager.OnLeft();
  }
  if (GetKey(olc::Key::RIGHT).bPressed) {
    menu_manager.OnRight();
  }
  if (GetKey(olc::Key::ENTER).bPressed || GetKey(olc::Key::SPACE).bPressed) {
    std::cout << "Enter/space  pressed\n";
    command = menu_manager.OnConfirm();
  }
  if (GetKey(olc::Key::ESCAPE).bPressed) {
    menu_manager.OnBack();
  }

  if (command != nullptr) {
    auto menu_id = MenuId(command->GetID());
    switch (menu_id) {
      case MenuId::Back:
        menu_manager.OnBack();
        break;

      case MenuId::Play:
        state_ = State::StartGame;
        menu_manager.Close();
        break;

      case MenuId::VeryEasy:
        difficulty_ = Difficulty::VeryEasy;
        menu_manager.OnBack();
        break;

      case MenuId::Easy:
        difficulty_ = Difficulty::Easy;
        menu_manager.OnBack();
        break;

      case MenuId::Intermediate:
        difficulty_ = Difficulty::Intermediate;
        menu_manager.OnBack();
        break;

      case MenuId::Hard:
        difficulty_ = Difficulty::Hard;
        menu_manager.OnBack();
        break;

      case MenuId::Expert:
        difficulty_ = Difficulty::Expert;
        menu_manager.OnBack();
        break;

      case MenuId::MinVerySmall:
      case MenuId::MinSmall:
      case MenuId::MinMedium:
      case MenuId::MinLarge:
      case MenuId::MinHuge:
        min_board_size_idx_ = +menu_id - +MenuId::MinVerySmall;
        max_board_size_idx_ =
            std::max(min_board_size_idx_, max_board_size_idx_);
        menu_manager.OnBack();
        break;

      case MenuId::MaxVerySmall:
      case MenuId::MaxSmall:
      case MenuId::MaxMedium:
      case MenuId::MaxLarge:
      case MenuId::MaxHuge:
        max_board_size_idx_ = +menu_id - +MenuId::MaxVerySmall;
        min_board_size_idx_ =
            std::min(min_board_size_idx_, max_board_size_idx_);
        menu_manager.OnBack();
        break;

      case MenuId::Settings:
        throw "Expected Unreachable";
        break;
    }
  }
  return true;
}

bool
Illum::OnUserCreate() {
  bool result = create_menu();

  return result;
}

bool
Illum::OnUserUpdate(float elapsed_time) {
  switch (state_) {
    case State::Playing:
      update_game();
      break;

    case State::Menu:
      update_menu();
      break;

    case State::StartGame:
      start_game();
      break;

    case State::Exit:
      std::cerr << "Exiting\n";
      return false;
  }

  render();

  // keep those fans from spinning needlessly
  std::this_thread::sleep_for(
      std::chrono::duration<float>(RENDER_INTERVAL - elapsed_time));
  return true;
}

bool
Illum::render() {
  Clear(olc::DARK_BLUE);

  // Draw Boundary
  DrawLine(10, 10, 502, 10, olc::YELLOW);
  DrawLine(10, 10, 10, 470, olc::YELLOW);
  DrawLine(502, 10, 502, 470, olc::YELLOW);
  DrawLine(10, 470, 502, 470, olc::YELLOW);

  DrawString(
      30,
      30,
      fmt::format("Bulbs Remaining: {}", bulbs_in_solution_ - bulbs_played_),
      (bulbs_played_ < bulbs_in_solution_) ||
              ((bulbs_played_ == bulbs_in_solution_) && position_.is_solved())
          ? olc::WHITE
          : olc::RED);

  switch (state_) {
    case State::Menu:
      DrawString(200, 100, fmt::format("Difficulty: {}", difficulty_));
      //      DrawString(200 + 13 * 8, 100, to_string(difficulty_));
      DrawString(200,
                 110,
                 fmt::format("Board Size: {}x{}",
                             BOARD_SIZES[min_board_size_idx_],
                             BOARD_SIZES[max_board_size_idx_]));

      menu_manager.Draw(menu_sprite_.get(), {40, 40});
      return true;

    case State::Playing:
      return render_game();

    case State::StartGame:
      return true;
  }
  std::cerr << "returning false " << __LINE__ << std::endl;
  std::cerr << "State=" << +state_ << "\n";
  return false;
}

void
Illum::on_state_change(model::Action    action,
                       model::CellState prev_state,
                       model::CellState to_state,
                       model::Coord     coord) {
  if (action == model::Action::Remove && prev_state == model::CellState::Bulb) {
    bulbs_played_--;
    position_.reset(model_.get_underlying_board(),
                    solver::PositionBoard::ResetPolicy::KEEP_ERRORS);
  }
  else {
    if (action == model::Action::Add && to_state == model::CellState::Bulb) {
      ++bulbs_played_;
    }
    position_.apply_move(
        model::SingleMove{action, prev_state, to_state, coord});
  }
}

bool
Illum::start_game() {
  auto dist    = std::uniform_int_distribution<int>(min_board_size_idx_,
                                                 max_board_size_idx_);
  int  height  = BOARD_SIZES[dist(twister_rng_)];
  int  width   = BOARD_SIZES[dist(twister_rng_)];
  model_       = levels::BasicWallLayout{}.create(twister_rng_, height, width);
  tile_width_  = ScreenWidth() / (model_.width() + 2 * COL_PADDING);
  tile_height_ = ScreenHeight() /
                 (model_.height() + ROW_PADDING_ABOVE + ROW_PADDING_BELOW);
  state_ = State::Playing;

  position_.reset(model_.get_underlying_board());

  bulbs_in_solution_ = 0;
  bulbs_played_      = 0;

  auto solution = solver::solve(position_.board());
  solution.board().visit_board(
      [&](auto, auto cell) { bulbs_in_solution_ += is_bulb(cell); });

  model_.set_state_change_handler(std::make_unique<StateChange>(*this));

  std::cout << model_.get_underlying_board() << std::endl;
  return true;
}

model::Coord
Illum::get_mouse_pos_as_tile_coord() const {
  // there is 1 tile of padding as margins on left/right sides.
  int col_coord = GetMouseX() / tile_width_ - COL_PADDING;
  int row_coord = GetMouseY() / tile_height_ - ROW_PADDING_ABOVE;

  return {row_coord, col_coord};
}

void
Illum::play_tile_at(model::CellState play_tile) {
  model::Coord coord = get_mouse_pos_as_tile_coord();
  if (auto cell = position_.get_opt_cell(coord)) {
    std::cerr << *cell << std::endl;
    if (is_empty(*cell)) {
      model_.add(play_tile, coord);
    }
    else {
      model_.remove(coord);
    }
  }
}

bool
Illum::update_game() {
  if (GetMouse(olc::Mouse::LEFT).bPressed) {
    play_tile_at(model::CellState::Bulb);
  }
  else if (GetMouse(olc::Mouse::RIGHT).bPressed) {
    play_tile_at(model::CellState::Mark);
  }

  if (position_.is_solved() && bulbs_played_ == bulbs_in_solution_) {
    state_ = State::StartGame;
  }

  return true;
}

bool
Illum::render_game() {
  position_.visit_board([&](model::Coord coord, model::CellState cell) {
    int x_px = (coord.col_ + COL_PADDING) * tile_width_;
    int y_px = (coord.row_ + ROW_PADDING_ABOVE) * tile_height_;

    using enum model::CellState;
    switch (cell) {
      case Empty:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::BLACK);
        break;
      case Wall0:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_GREY);
        DrawRect(x_px, y_px, tile_width_, tile_height_, olc::BLACK);
        break;
      case Wall1:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_GREY);
        DrawString(x_px + tile_width_ / 2 - HALF_CHAR_PXLS,
                   y_px + tile_height_ / 2 - HALF_CHAR_PXLS,
                   "1");
        DrawRect(x_px, y_px, tile_width_, tile_height_, olc::BLACK);
        break;
      case Wall2:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_GREY);
        DrawString(x_px + tile_width_ / 2 - HALF_CHAR_PXLS,
                   y_px + tile_height_ / 2 - HALF_CHAR_PXLS,
                   "2");
        DrawRect(x_px, y_px, tile_width_, tile_height_, olc::BLACK);
        break;
      case Wall3:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_GREY);
        DrawString(x_px + tile_width_ / 2 - HALF_CHAR_PXLS,
                   y_px + tile_height_ / 2 - HALF_CHAR_PXLS,
                   "3");
        DrawRect(x_px, y_px, tile_width_, tile_height_, olc::BLACK);
        break;
      case Wall4:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_GREY);
        DrawString(x_px + tile_width_ / 2 - HALF_CHAR_PXLS,
                   y_px + tile_height_ / 2 - HALF_CHAR_PXLS,
                   "4");
        DrawRect(x_px, y_px, tile_width_, tile_height_, olc::BLACK);
        break;
      case Bulb:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_YELLOW);
        FillCircle(x_px + tile_width_ / 2,
                   y_px + tile_height_ / 2,
                   std::min(tile_height_, tile_width_) / 3,
                   olc::YELLOW);
        break;
      case Illum:
        FillRect(x_px, y_px, tile_width_, tile_height_, olc::DARK_YELLOW);
        break;

      case Mark:
        FillRect(x_px + tile_width_ / 4,
                 y_px + tile_height_ / 4,
                 tile_width_ / 2,
                 tile_height_ / 2,
                 olc::BLUE);
        break;
    }
  });
  return true;
}

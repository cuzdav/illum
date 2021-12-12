#pragma once
#include "olcPixelGameEngine.h"
#include <functional>
#include <memory>

// my own Simple olc sprite-based Button class

namespace illum::olc {

class Button {
public:
  using Callback = std::function<void()>;

  ::olc::vi2d position;
  ::olc::vi2d size;
  ::olc::vf2d scale;

  Button(::olc::PixelGameEngine * engine,
         Callback                 on_click_handler,
         std::string const &      button_sprite_filename,
         ::olc::vi2d              pos,
         ::olc::vi2d              size)
      : position(pos)
      , engine_(engine)
      , sprite_(button_sprite_filename)
      , decal_(&sprite_)
      , size(size)
      , on_click_handler_(std::forward<Callback>(on_click_handler)) {
    scale = {float(size.x) / sprite_.width, float(size.y) / sprite_.height};
  }

  enum class State { UP, DOWN };

  void
  update() {
    if (bool const mouse_over = mouse_within_bounds()) {
      auto const & mouse_state = engine_->GetMouse(::olc::Mouse::LEFT);

      if (mouse_state.bReleased) {
        on_click_handler_();
      }
      state_ = mouse_state.bHeld ? State::DOWN : State::UP;
    }
    else {
      state_ = State::UP;
    }
  }

  void
  render() {
    auto tint = state_ == State::DOWN ? ::olc::RED : ::olc::WHITE;
    engine_->DrawDecal(position, &decal_, scale, tint);
  }

private:
  bool
  mouse_within_bounds() {
    int const mouse_x = engine_->GetMouseX();
    int const mouse_y = engine_->GetMouseY();
    return (mouse_x >= position.x && mouse_x <= position.x + size.x &&
            mouse_y >= position.y && mouse_y <= position.y + size.y);
  }

private:
  ::olc::PixelGameEngine * engine_;
  ::olc::Sprite            sprite_;
  ::olc::Decal             decal_;
  State                    state_ = State::UP;
  std::function<void()>    on_click_handler_;
};

} // namespace illum::olc

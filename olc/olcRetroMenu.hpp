/*
        olcPGEX_PopUP.h

        +-------------------------------------------------------------+
        |         OneLoneCoder Pixel Game Engine Extension            |
        |                Retro PopUP Menu 1.0                         |
        +-------------------------------------------------------------+

        What is this?
        ~~~~~~~~~~~~~
        This is an extension to the olcPixelGameEngine, which provides
        a quick and easy to use, flexible, skinnable context pop-up
        menu system.

        License (OLC-3)
        ~~~~~~~~~~~~~~~

        Copyright 2018 - 2020 OneLoneCoder.com

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions
        are met:

        1. Redistributions or derivations of source code must retain the above
        copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions or derivative works in binary form must reproduce
        the above copyright notice. This list of conditions and the following
        disclaimer must be reproduced in the documentation and/or other
        materials provided with the distribution.

        3. Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
        "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
        LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
        A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
        HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
        SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
        LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
        DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
        THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
        OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        Links
        ~~~~~
        YouTube:	https://www.youtube.com/javidx9
        Discord:	https://discord.gg/WhwHUMV
        Twitter:	https://www.twitter.com/javidx9
        Twitch:		https://www.twitch.tv/javidx9
        GitHub:		https://www.github.com/onelonecoder
        Homepage:	https://www.onelonecoder.com

        Author
        ~~~~~~
        David Barr, aka javidx9, ©OneLoneCoder 2019, 2020
*/

/*
        Example
        ~~~~~~~

        #define OLC_PGEX_POPUPMENU
        #include "olcPGEX_PopUPMenu.h"

        NOTE: Requires a 9-patch sprite, by default each patch is
        8x8 pixels, patches are as follows:

        | PANEL TL | PANEL T | PANEL TR | SCROLL UP   | CURSOR TL | CURSOR TR |
        | PANEL L  | PANEL M | PANEL R  | SUBMENU     | CURSOR BL | CURSOR BR |
        | PANEL BL | PANEL B | PANEL BR | SCROLL DOWN | UNUSED    | UNUSED    |

        You can find an example sprite here:
        https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/RetroMenu.png

        Constructing A Menu
        ~~~~~~~~~~~~~~~~~~~

        // Declaration (presumably inside class)
        olc::popup::Menu m;

        // Construction (root menu is a 1x5 table)
        m.SetTable(1, 5);

        // ADD first item  to root menu (A 1x5 submenu)
        m["Menu1"].SetTable(1, 5);

        // ADD items to first item
        m["Menu1"]["Item1"];
        m["Menu1"]["Item2"];

        // ADD a 4x3 submenu
        m["Menu1"]["Item3"].SetTable(4, 3);
        m["Menu1"]["Item3"]["Option1"];
        m["Menu1"]["Item3"]["Option2"];

        // Set properties of specific item
        m["Menu1"]["Item3"]["Option3"].Enable(false);
        m["Menu1"]["Item3"]["Option4"];
        m["Menu1"]["Item3"]["Option5"];
        m["Menu1"]["Item4"];

        // ADD second item to root menu
        m["Menu2"].SetTable(3, 3);
        m["Menu2"]["Item1"];
        m["Menu2"]["Item2"].SetID(1001).Enable(true);
        m["Menu2"]["Item3"];

        // Construct the menu structure
        m.Build();


        Displaying a Menu
        ~~~~~~~~~~~~~~~~~

        // Declaration of menu manager (presumably inside class)
        olc::popup::Manager man;

        // Set the Menu object to the MenuManager (call once per pop)
        man.Open(&m);

        // Draw Menu at position (30, 30), using "patch sprite"
        man.Draw(sprGFX, { 30,30 });


        Interacting with menu
        ~~~~~~~~~~~~~~~~~~~~~

        // Send key events to menu
        if (GetKey(olc::Key::UP).bPressed)    man.OnUP();
        if (GetKey(olc::Key::DOWN).bPressed)  man.OnDOWN();
        if (GetKey(olc::Key::LEFT).bPressed)  man.OnLEFT();
        if (GetKey(olc::Key::RIGHT).bPressed) man.OnRIGHT();
        if (GetKey(olc::Key::Z).bPressed)     man.OnBack();

        // "Confirm/Action" Key does something, if it returns non-null
        // then a menu item has been selected. The specific item will
        // be returned
        olc::popup::Menu* command = nullptr;
        if (GetKey(olc::Key::SPACE).bPressed) command = man.OnConfirm();
        if (command != nullptr)
        {
                std::string sLastAction =
                "Selected: " + command->GetName() +
                " ID: " + std::to_string(command->GetID());

                // Optionally close menu?
                man.Close();
        }

*/

#pragma once

#include "olcPixelGameEngine.h"
#include <cstdint>
#include <unordered_map>

namespace olc {
namespace popup {
constexpr int32_t nPatch = 8;

class Menu {
public:
  Menu() = default;
  Menu(const std::string n);

  Menu & SetTable(int32_t nColumns, int32_t nRows);
  Menu & SetID(int32_t id);
  Menu & Enable(bool b);

  int32_t       GetID();
  std::string & GetName();
  bool          Enabled();
  bool          HasChildren();
  olc::vi2d     GetSize();
  olc::vi2d &   GetCursorPosition();
  Menu &        operator[](const std::string & name);
  void          Build();
  void          DrawSelf(olc::PixelGameEngine & pge,
                         olc::Sprite *          sprGFX,
                         olc::vi2d              vScreenOffset);
  void          ClampCursor();
  void          OnUP();
  void          OnDOWN();
  void          OnLEFT();
  void          OnRIGHT();
  Menu *        OnConfirm();
  Menu *        GetSelectedItem();

protected:
  int32_t                                 nID        = -1;
  olc::vi2d                               vCellTable = {1, 0};
  std::unordered_map<std::string, size_t> itemPointer;
  std::vector<olc::popup::Menu>           items;
  olc::vi2d                               vSizeInPatches = {0, 0};
  olc::vi2d                               vCellSize      = {0, 0};
  olc::vi2d                               vCellPadding   = {2, 0};
  olc::vi2d                               vCellCursor    = {0, 0};
  int32_t                                 nCursorItem    = 0;
  int32_t                                 nTopVisibleRow = 0;
  int32_t                                 nTotalRows     = 0;
  const olc::vi2d                         vPatchSize     = {nPatch, nPatch};
  std::string                             sName;
  olc::vi2d                               vCursorPos = {0, 0};
  bool                                    bEnabled   = true;
};

class Manager : public olc::PGEX {
public:
  Manager();
  bool   IsOpen() const;
  void   Open(Menu * mo);
  void   Close();
  void   OnUP();
  void   OnDOWN();
  void   OnLEFT();
  void   OnRIGHT();
  void   OnBack();
  Menu * OnConfirm();
  void   Draw(olc::Sprite * sprGFX, olc::vi2d vScreenOffset);

private:
  std::list<Menu *> panels;
};

} // namespace popup
} // namespace olc

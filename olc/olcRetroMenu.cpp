#include <cstdint>

#include "olcRetroMenu.hpp"

namespace olc {
namespace popup {

Menu::Menu(const std::string n) { sName = n; }

Menu &
Menu::SetTable(int32_t nColumns, int32_t nRows) {
  vCellTable = {nColumns, nRows};
  return *this;
}

Menu &
Menu::SetID(int32_t id) {
  nID = id;
  return *this;
}

Menu &
Menu::Enable(bool b) {
  bEnabled = b;
  return *this;
}

int32_t
Menu::GetID() {
  return nID;
}

std::string &
Menu::GetName() {
  return sName;
}

bool
Menu::Enabled() {
  return bEnabled;
}

bool
Menu::HasChildren() {
  return !items.empty();
}

olc::vi2d
Menu::GetSize() {
  return {int32_t(sName.size()), 1};
}

olc::vi2d &
Menu::GetCursorPosition() {
  return vCursorPos;
}

Menu &
Menu::operator[](const std::string & name) {
  if (itemPointer.count(name) == 0) {
    itemPointer[name] = items.size();
    items.push_back(Menu(name));
  }

  return items[itemPointer[name]];
}

void
Menu::Build() {
  // Recursively build all children, so they can determine their size, use
  // that size to indicate cell sizes if this object contains more than
  // one item
  for (auto & m : items) {
    if (m.HasChildren()) {
      m.Build();
    }

    // Longest child name determines cell width
    vCellSize.x = std::max(m.GetSize().x, vCellSize.x);
    vCellSize.y = std::max(m.GetSize().y, vCellSize.y);
  }

  // Adjust size of this object (in patches) if it were rendered as a panel
  vSizeInPatches.x =
      vCellTable.x * vCellSize.x + (vCellTable.x - 1) * vCellPadding.x + 2;
  vSizeInPatches.y =
      vCellTable.y * vCellSize.y + (vCellTable.y - 1) * vCellPadding.y + 2;

  // Calculate how many rows this item has to hold
  nTotalRows = (items.size() / vCellTable.x) +
               (((items.size() % vCellTable.x) > 0) ? 1 : 0);
}

void
Menu::DrawSelf(olc::PixelGameEngine & pge,
               olc::Sprite *          sprGFX,
               olc::vi2d              vScreenOffset) {
  // === Draw Panel

  // Record current pixel mode user is using
  olc::Pixel::Mode currentPixelMode = pge.GetPixelMode();
  pge.SetPixelMode(olc::Pixel::MASK);

  // Draw Panel & Border
  olc::vi2d vPatchPos = {0, 0};
  for (vPatchPos.x = 0; vPatchPos.x < vSizeInPatches.x; vPatchPos.x++) {
    for (vPatchPos.y = 0; vPatchPos.y < vSizeInPatches.y; vPatchPos.y++) {
      // Determine position in screen space
      olc::vi2d vScreenLocation = vPatchPos * nPatch + vScreenOffset;

      // Calculate which patch is needed
      olc::vi2d vSourcePatch = {0, 0};
      if (vPatchPos.x > 0)
        vSourcePatch.x = 1;
      if (vPatchPos.x == vSizeInPatches.x - 1)
        vSourcePatch.x = 2;
      if (vPatchPos.y > 0)
        vSourcePatch.y = 1;
      if (vPatchPos.y == vSizeInPatches.y - 1)
        vSourcePatch.y = 2;

      // Draw Actual Patch
      pge.DrawPartialSprite(
          vScreenLocation, sprGFX, vSourcePatch * nPatch, vPatchSize);
    }
  }

  // === Draw Panel Contents
  olc::vi2d vCell = {0, 0};
  vPatchPos       = {1, 1};

  // Work out visible items
  int32_t nTopLEFTItem     = nTopVisibleRow * vCellTable.x;
  int32_t nBottomRIGHTItem = vCellTable.y * vCellTable.x + nTopLEFTItem;

  // Clamp to size of child item vector
  nBottomRIGHTItem      = std::min(int32_t(items.size()), nBottomRIGHTItem);
  int32_t nVisibleItems = nBottomRIGHTItem - nTopLEFTItem;

  // Draw Scroll MARKers (if required)
  if (nTopVisibleRow > 0) {
    vPatchPos                 = {vSizeInPatches.x - 2, 0};
    olc::vi2d vScreenLocation = vPatchPos * nPatch + vScreenOffset;
    olc::vi2d vSourcePatch    = {3, 0};
    pge.DrawPartialSprite(
        vScreenLocation, sprGFX, vSourcePatch * nPatch, vPatchSize);
  }

  if ((nTotalRows - nTopVisibleRow) > vCellTable.y) {
    vPatchPos                 = {vSizeInPatches.x - 2, vSizeInPatches.y - 1};
    olc::vi2d vScreenLocation = vPatchPos * nPatch + vScreenOffset;
    olc::vi2d vSourcePatch    = {3, 2};
    pge.DrawPartialSprite(
        vScreenLocation, sprGFX, vSourcePatch * nPatch, vPatchSize);
  }

  // Draw Visible Items
  for (int32_t i = 0; i < nVisibleItems; i++) {
    // Cell location
    vCell.x = i % vCellTable.x;
    vCell.y = i / vCellTable.x;

    // Patch location (including border offset and padding)
    vPatchPos.x = vCell.x * (vCellSize.x + vCellPadding.x) + 1;
    vPatchPos.y = vCell.y * (vCellSize.y + vCellPadding.y) + 1;

    // Actual screen location in pixels
    olc::vi2d vScreenLocation = vPatchPos * nPatch + vScreenOffset;

    // Display Item Header
    pge.DrawString(vScreenLocation,
                   items[nTopLEFTItem + i].sName,
                   items[nTopLEFTItem + i].bEnabled ? olc::WHITE
                                                    : olc::DARK_GREY);

    if (items[nTopLEFTItem + i].HasChildren()) {
      // Display Indicator that panel has a sub panel
      vPatchPos.x = vCell.x * (vCellSize.x + vCellPadding.x) + 1 + vCellSize.x;
      vPatchPos.y = vCell.y * (vCellSize.y + vCellPadding.y) + 1;
      olc::vi2d vSourcePatch = {3, 1};
      vScreenLocation        = vPatchPos * nPatch + vScreenOffset;
      pge.DrawPartialSprite(
          vScreenLocation, sprGFX, vSourcePatch * nPatch, vPatchSize);
    }
  }

  // Calculate cursor position in screen space in case system draws it
  vCursorPos.x = (vCellCursor.x * (vCellSize.x + vCellPadding.x)) * nPatch +
                 vScreenOffset.x - nPatch;
  vCursorPos.y =
      ((vCellCursor.y - nTopVisibleRow) * (vCellSize.y + vCellPadding.y)) *
          nPatch +
      vScreenOffset.y + nPatch;
}

void
Menu::ClampCursor() {
  // Find item in children
  nCursorItem = vCellCursor.y * vCellTable.x + vCellCursor.x;

  // Clamp Cursor
  if (nCursorItem >= int32_t(items.size())) {
    vCellCursor.y = (items.size() / vCellTable.x);
    vCellCursor.x = (items.size() % vCellTable.x) - 1;
    nCursorItem   = items.size() - 1;
  }
}

void
Menu::OnUP() {
  vCellCursor.y--;
  if (vCellCursor.y < 0)
    vCellCursor.y = 0;

  if (vCellCursor.y < nTopVisibleRow) {
    nTopVisibleRow--;
    if (nTopVisibleRow < 0)
      nTopVisibleRow = 0;
  }

  ClampCursor();
}

void
Menu::OnDOWN() {
  vCellCursor.y++;
  if (vCellCursor.y == nTotalRows)
    vCellCursor.y = nTotalRows - 1;

  if (vCellCursor.y > (nTopVisibleRow + vCellTable.y - 1)) {
    nTopVisibleRow++;
    if (nTopVisibleRow > (nTotalRows - vCellTable.y))
      nTopVisibleRow = nTotalRows - vCellTable.y;
  }

  ClampCursor();
}

void
Menu::OnLEFT() {
  vCellCursor.x--;
  if (vCellCursor.x < 0)
    vCellCursor.x = 0;
  ClampCursor();
}

void
Menu::OnRIGHT() {
  vCellCursor.x++;
  if (vCellCursor.x == vCellTable.x)
    vCellCursor.x = vCellTable.x - 1;
  ClampCursor();
}

Menu *
Menu::OnConfirm() {
  // Check if selected item has children
  if (items[nCursorItem].HasChildren()) {
    return &items[nCursorItem];
  }
  else
    return this;
}

Menu *
Menu::GetSelectedItem() {
  return &items[nCursorItem];
}

// =====================================================================

Manager::Manager() {}

void
Manager::Open(Menu * mo) {
  Close();
  panels.push_back(mo);
}

void
Manager::Close() {
  panels.clear();
}

bool
Manager::IsOpen() const {
  return (!panels.empty());
}

void
Manager::OnUP() {
  if (!panels.empty())
    panels.back()->OnUP();
}

void
Manager::OnDOWN() {
  if (!panels.empty())
    panels.back()->OnDOWN();
}

void
Manager::OnLEFT() {
  if (!panels.empty())
    panels.back()->OnLEFT();
}

void
Manager::OnRIGHT() {
  if (!panels.empty())
    panels.back()->OnRIGHT();
}

void
Manager::OnBack() {
  if (!panels.empty())
    panels.pop_back();
}

Menu *
Manager::OnConfirm() {
  if (panels.empty())
    return nullptr;

  Menu * next = panels.back()->OnConfirm();
  if (next == panels.back()) {
    if (panels.back()->GetSelectedItem()->Enabled())
      return panels.back()->GetSelectedItem();
  }
  else {
    if (next->Enabled())
      panels.push_back(next);
  }

  return nullptr;
}

void
Manager::Draw(olc::Sprite * sprGFX, olc::vi2d vScreenOffset) {
  if (panels.empty())
    return;

  // Draw Visible Menu System
  for (auto & p : panels) {
    p->DrawSelf(*pge, sprGFX, vScreenOffset);
    vScreenOffset += {10, 10};
  }

  // Draw Cursor
  olc::Pixel::Mode currentPixelMode = pge->GetPixelMode();
  pge->SetPixelMode(olc::Pixel::ALPHA);
  pge->DrawPartialSprite(panels.back()->GetCursorPosition(),
                         sprGFX,
                         olc::vi2d(4, 0) * nPatch,
                         {nPatch * 2, nPatch * 2});
  pge->SetPixelMode(currentPixelMode);
}
} // namespace popup
} // namespace olc

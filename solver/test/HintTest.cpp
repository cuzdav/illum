#include "Hint.hpp"

#include "ASCIILevelCreator.hpp"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock-more-matchers.h"
#include "gtest/gtest.h"

namespace solver::test {

using namespace ::testing;
using namespace model;

TEST(HintTest, simple_forced_isolated_cell) {
  ASCIILevelCreator creator;
  creator("0.0");
  creator(".0.");
  BasicBoard board;
  creator.finished(&board);

  Hint hint = Hint::create(board);

  EXPECT_THAT(
      hint.next_moves(),
      ElementsAre(Eq(AnnotatedMove{
          SingleMove{
              Action::ADD, CellState::EMPTY, CellState::BULB, Coord{0, 1}},
          DecisionType::ISOLATED_EMPTY_SQUARE,
          MoveMotive::FORCED})));
  EXPECT_THAT(hint.explain_steps(), IsEmpty());
}

TEST(HintTest, simple_forced_isolated_mark) {
  ASCIILevelCreator creator;
  creator("0X0");
  creator("*++");
  creator("+..");
  BasicBoard board;
  creator.finished(&board);

  Hint hint = Hint::create(board);

  EXPECT_THAT(
      hint.next_moves(),
      ElementsAre(Eq(AnnotatedMove{
          SingleMove{
              Action::ADD, CellState::EMPTY, CellState::BULB, Coord{2, 1}},
          DecisionType::ISOLATED_MARK,
          MoveMotive::FORCED,
          Coord{0, 1}})));
  EXPECT_THAT(hint.explain_steps(), IsEmpty());
}

TEST(HintTest, simple_forced_satisfied_wall) {
  ASCIILevelCreator creator;
  creator("..+");
  creator(".2*");
  creator("+*+");

  BasicBoard board;
  creator.finished(&board);

  Hint hint = Hint::create(board);

  EXPECT_THAT(
      hint.next_moves(),
      ElementsAre(
          Eq(AnnotatedMove{
              SingleMove{
                  Action::ADD, CellState::EMPTY, CellState::MARK, Coord{0, 1}},
              DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
              MoveMotive::FORCED,
              Coord{1, 1}}),
          Eq(AnnotatedMove{
              SingleMove{
                  Action::ADD, CellState::EMPTY, CellState::MARK, Coord{1, 0}},
              DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
              MoveMotive::FORCED,
              Coord{1, 1}})));
  EXPECT_THAT(hint.explain_steps(), IsEmpty());
}

TEST(HintTest, simple_forced_deps_equal_open_faces) {
  ASCIILevelCreator creator;
  creator("..0");
  creator(".20");
  creator("000");

  BasicBoard board;
  creator.finished(&board);

  Hint hint = Hint::create(board);

  EXPECT_THAT(
      hint.next_moves(),
      ElementsAre(
          Eq(AnnotatedMove{
              SingleMove{
                  Action::ADD, CellState::EMPTY, CellState::BULB, Coord{0, 1}},
              DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
              MoveMotive::FORCED,
              Coord{1, 1}}),
          Eq(AnnotatedMove{
              SingleMove{
                  Action::ADD, CellState::EMPTY, CellState::BULB, Coord{1, 0}},
              DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
              MoveMotive::FORCED,
              Coord{1, 1}})));
  EXPECT_THAT(hint.explain_steps(), IsEmpty());
}

TEST(HintTest, speculation_single_dep_wall_near_edge) {
  ASCIILevelCreator creator;
  creator("0..");
  creator("..1");
  creator("...");

  BasicBoard board;
  creator.finished(&board);

  Hint hint = Hint::create(board);

  EXPECT_THAT(
      hint.next_moves(),
      ElementsAre(Eq(AnnotatedMove{
          SingleMove{
              Action::ADD, CellState::EMPTY, CellState::MARK, Coord{1, 1}},
          DecisionType::MARK_CANNOT_BE_ILLUMINATED,
          MoveMotive::FORCED,
          Coord{0, 2}})));

  EXPECT_THAT(hint.explain_steps(), IsEmpty());
}

} // namespace solver::test

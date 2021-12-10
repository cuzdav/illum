Illum is a logic game.  The goal of each level is to illuminate every
non-wall tile without breaking any requirements.

You can place (or remove) bulbs or marks on the board.  Bulbs radiate
light in all 4 directions outward, until they hit a wall.  Marks are
indications that you believe a bulb does NOT go in the cell, and can
help you keep track of such cells.  (These are similar in concept to
the pencil-tip marks players put on sudoku boards.) Eventually all
marks must be illuminated by a bulb nearby.

The requirements are:

1) Two bulbs cannot be in the same row or column if they can see each other.

2) Some walls require an exact number of bulbs to be adjacent to them.
Under- or over-populating them is an error.  They will indicate their
number of required bulbs.

3) There is only one unique solution.  If you create a situation where
two or more solutions are possible, you have made an error along the
way.

There is always enough information to make a deduction to discover the
next move.  You should *never* have to guess. On simple boards the
next move is straight-forward, but on difficult boards some process of
elimination may be required, and may possibly require multi-step
(mental) lookahead.

Deductions are made by finding either forced moves or contradictions.

A forced move is simply a consequence of the rules and the current
board position.  A few examples:

* if a wall requires 4 bulbs around it, then all four sides must be bulbs.

* if a wall requires 1 bulb and has 3 sides around it blocked, the
free side must be a bulb.

* if there is only one place to put a bulb that can illuminate a cell,
then that cell must be a bulb.

* if placing a bulb would cause a wall to have too many bulbs around
it, then that cell must _not_ be a bulb.  (And should be a mark, to
help indicate this.)


Good luck, and enjoy.

Feedback, suggestions, bug reports, pull requests, etc., all welcome.

Chris





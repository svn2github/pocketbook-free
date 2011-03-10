
#define DOTTED 0xFF000000


#ifdef BLACKBOX_GAME
// COL_BACKGROUND,COL_COVER,COL_LOCK,COL_TEXT,COL_FLASHTEXT,COL_HIGHLIGHT,
// COL_LOWLIGHT,COL_GRID,COL_BALL,COL_WRONG,COL_BUTTON,COL_CURSOR
int inkcolors[12] = {WHITE, LGRAY, DGRAY, BLACK, LGRAY, LGRAY, DGRAY, DGRAY, DGRAY, DGRAY, LGRAY, BLACK};

char *HelpMessage="A number of balls are hidden in a arena. You have to deduce the positions of the balls by firing lasers positioned at the edges of the arena and observing how their beams are deflected.\
 Beams will travel straight from their origin until they hit the opposite side of the arena, unless affected by balls in one of the following ways:\n\
  * A beam that hits a ball head-on is absorbed and will never re-emerge.\n\
  * A beam with a ball to its front-left square gets deflected 90 degrees to the right.\n\
  * A beam with a ball to its front-right square gets similarly deflected to the left.\n\
  * A beam that would re-emerge from its entry location is considered to be 'reflected'.\n\
  * A beam which would get deflected before entering the arena by a ball to the front-left or front-right of its entry point is also considered to be 'reflected'.\n\
Beams that are reflected appear as a 'R'; beams that hit balls head-on appear as 'H'. Otherwise, a number appears at the firing point and the location where the beam emerges.\n\
You can place guesses as to the location of the balls, based on the entry and exit patterns of the beams; once you have placed enough balls a button appears enabling you to have your guesses checked.\n\
Black Box controls:\n\
Use \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid. Pressing the OK key will fire a laser or add a new ball-location guess, and pressing NEXTPAGE (or PLUS) key will lock a cell, row, or column.\n\
When an appropriate number of balls have been guessed, a button will appear at the top-left corner of the grid; clicking that will check your guesses.";
#endif

#ifdef BRIDGES_GAME
// COL_BACKGROUND,COL_FOREGROUND,COL_HIGHLIGHT,COL_LOWLIGHT,
// COL_SELECTED,COL_MARK,COL_HINT,COL_GRID,COL_WARNING,COL_CURSOR
int inkcolors[10] = {WHITE, BLACK, LGRAY, DGRAY, LGRAY, DGRAY, DGRAY, BLACK, LGRAY, LGRAY};
char *HelpMessage="You have a set of islands distributed across the playing area. Each island contains a number. Your aim is to connect the islands together with bridges, in such a way that:\n\
    * Bridges run horizontally or vertically.\n\
    * The number of bridges terminating at any island is equal to the number written in that island.\n\
    * Two bridges may run in parallel between the same two islands, but no more than two may do so.\n\
    * No bridge crosses another bridge.\n\
    * All the islands are connected together.\n\n\
Bridges controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid: if possible the cursor will always move orthogonally, otherwise it will move towards the nearest island to the indicated direction. Pressing the OK key followed by a \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 key will lay a bridge in that direction (if available); pressing the NEXTPAGE (or PLUS) key followed by a \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 key will lay a 'non-bridge' marker.\n\
You can mark an island as finished by pressing the OK key twice.\n\
";
#endif

#ifdef CUBE_GAME
// COL_BACKGROUND,COL_BORDER,COL_BLUE
int inkcolors[3] = {WHITE, BLACK, LGRAY};
char *HelpMessage="You have a grid of 16 squares, six of which are gray; on one square rests a cube. Your move is to use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to roll the cube through 90 degrees so that it moves to an adjacent square. If you roll the cube on to a gray square, the gray square is picked up on one face of the cube; if you roll a gray face of the cube on to a non-gray square, the blueness is put down again. (In general, whenever you roll the cube, the two faces that come into contact swap colours.) Your job is to get all six gray squares on to the six faces of the cube at the same time. Count your moves and try to do it in as few as possible.\n\n\
Cube controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to roll the cube on its square grid in the four cardinal directions. On the triangular grids, the mapping of arrow keys to directions is more approximate. Vertical movement is disallowed where it doesn't make sense.";
#endif

#ifdef DOMINOSA_GAME
// COL_BACKGROUND,COL_TEXT,COL_DOMINO,COL_DOMINOCLASH,COL_DOMINOTEXT,COL_EDGE
int inkcolors[6] = {WHITE, BLACK, BLACK, LGRAY, WHITE, DGRAY};
char *HelpMessage="A normal set of dominoes - that is, one instance of every (unordered) pair of numbers from 0 to 6 - has been arranged irregularly into a rectangle; then the number in each square has been written down and the dominoes themselves removed. Your task is to reconstruct the pattern by arranging the set of dominoes to match the provided array of numbers.\n\n\
Dominosa controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move a cursor around the grid. When the cursor is half way between two adjacent numbers, pressing the OK key will place a domino covering those numbers, or pressing the NEXTPAGE (or PLUS) key bar will lay a line between the two squares. Repeating either action removes the domino or line.";
#endif

#ifdef FIFTEEN_GAME
// COL_BACKGROUND,COL_TEXT,COL_HIGHLIGHT,COL_LOWLIGHT
int inkcolors[4] = {WHITE, BLACK, LGRAY, DGRAY};
char *HelpMessage="You have a 4x4 square grid; 15 squares contain numbered tiles, and the sixteenth is empty. Your move is to choose a tile next to the empty space, and slide it into the space. The aim is to end up with the tiles in numerical order, with the space in the bottom right (so that the top row reads 1,2,3,4 and the bottom row reads 13,14,15,space).\n\n\
Fifteen controls:\n\
The \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys will move a tile adjacent to the space in the direction indicated (moving the space in the opposite direction).";
#endif

#ifdef FILLING_GAME
// COL_BACKGROUND,COL_GRID,COL_HIGHLIGHT,COL_CORRECT,COL_ERROR,COL_USER,COL_CURSOR,
int inkcolors[7] = {WHITE, DGRAY, DGRAY, LGRAY, DOTTED, BLACK, BLACK};
char *HelpMessage="You have a grid of squares, some of which contain digits, and the rest of which are empty. Your job is to fill in digits in the empty squares, in such a way that each connected region of squares all containing the same digit has an area equal to that digit.\n\
('Connected region', for the purposes of this game, does not count diagonally separated squares as adjacent.)\n\
For example, it follows that no square can contain a zero, and that two adjacent squares can not both contain a one. No region has an area greater than 9 (because then its area would not be a single digit).\n\n\
Filling controls:\n\
Move around the grid with the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys; press OK to select a digit - this will fill the square containing the cursor with that number, or selecting 0 will clear it. You can also select multiple squares for numbering or clearing by using the NEXTPAGE (or PLUS) key, before selecting a digit to fill in the highlighted squares.";
#endif

#ifdef FLIP_GAME
// COL_BACKGROUND,COL_WRONG,COL_RIGHT,COL_GRID,COL_DIAG,COL_HINT,COL_CURSOR
int inkcolors[7] = {WHITE, DGRAY, WHITE, LGRAY, LGRAY, DGRAY, BLACK};
char *HelpMessage="You have a grid of squares, some light and some dark. Your aim is to light all the squares up at the same time. You can choose any square and flip its state from light to dark or dark to light, but when you do so, other squares around it change state as well.\n\
Each square contains a small diagram showing which other squares change when you flip it.\n\n\
Flip controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to choose a square and the OK key or NEXTPAGE (or PLUS) key to flip.";
#endif

#ifdef GALAXIES_GAME
// COL_BACKGROUND,COL_WHITEBG,COL_BLACKBG,COL_WHITEDOT,COL_BLACKDOT,
// COL_GRID,COL_EDGE,COL_ARROW,COL_CURSOR
int inkcolors[9] = {WHITE, DGRAY, BLACK, BLACK, LGRAY, BLACK, BLACK, LGRAY, DGRAY};
char *HelpMessage="You have a rectangular grid containing a number of dots. Your aim is to draw edges along the grid lines which divide the rectangle into regions in such a way that every region is 180\xC2\xBA rotationally symmetric, and contains exactly one dot which is located at its centre of symmetry.\n\n\
Galaxies controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid squares and lines. Pressing the OK key when over a grid line will draw or clear its edge. Pressing the OK key when over a dot will pick up an arrow, to be dropped the next time the OK key is pressed; this can also be used to move existing arrows around, removing them by dropping them on a dot or another arrow.";
#endif

#ifdef GUESS_GAME
// COL_BACKGROUND,COL_FRAME,COL_CURSOR,COL_FLASH,COL_HOLD,
// COL_EMPTY, /* must be COL_1 - 1 */
// COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10,
// COL_CORRECTPLACE, COL_CORRECTCOLOUR
int inkcolors[18] = {WHITE, LGRAY, BLACK, DGRAY, BLACK, LGRAY,
 DOTTED, DOTTED, DOTTED, DOTTED, DOTTED, DOTTED, DOTTED, DOTTED, DOTTED, DOTTED,
 WHITE, BLACK};
char *HelpMessage="You have a set of numbered pegs, and have to reproduce a predetermined sequence of them (chosen by the computer) within a certain number of guesses.\n\
Each guess gets marked with the number of correctly-numbered pegs in the correct places (in white), and also the number of correctly-numbered pegs in the wrong places (in black).\n\n\
Guess controls:\n\
The \xE2\x86\x91 and \xE2\x86\x93 keys can be used to select a peg number, the \xE2\x86\x90 and \xE2\x86\x92 keys to select a peg position, and the OK key to place a peg of the selected number in the chosen position. NEXTPAGE (or PLUS) key adds a hold marker.\n\
When the guess is complete, the smaller feedback pegs will be highlighted; moving the peg cursor to them with the \xE2\x86\x90 and \xE2\x86\x92 keys and pressing the OK key will mark the current guess, copy any held pegs to the next guess, and move the 'current guess' marker.\n\
If you correctly position all the pegs the solution will be displayed below.";
#endif

#ifdef INERTIA_GAME
// COL_BACKGROUND,COL_OUTLINE,COL_HIGHLIGHT,COL_LOWLIGHT,COL_PLAYER,
// COL_DEAD_PLAYER,COL_MINE,COL_GEM,COL_WALL,COL_HINT
int inkcolors[10] = {WHITE, DGRAY, LGRAY, DGRAY, DGRAY, WHITE, BLACK, LGRAY, LGRAY, LGRAY};
char *HelpMessage="You are a small gray ball sitting in a grid full of obstacles. Your aim is to collect all the gems without running into any mines.\n\
You can move the ball in any orthogonal or diagonal direction. Once the ball starts moving, it will continue until something stops it. A wall directly in its path will stop it (but if it is moving diagonally, it will move through a diagonal gap between two other walls without stopping). Also, some of the squares are 'stops'; when the ball moves on to a stop, it will stop moving no matter what direction it was going in. Gems do not stop the ball; it picks them up and keeps on going.\n\
Running into a mine is fatal. Even if you picked up the last gem in the same move which then hit a mine, the game will count you as dead rather than victorious.\n\n\
Inertia controls:\n\
You can move the ball in any of the eight directions using the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys (press OK to switch orthogonal/diagonal direction).";
#endif

#ifdef KEEN_GAME
// COL_BACKGROUND,COL_GRID,COL_USER,COL_HIGHLIGHT,COL_ERROR,COL_PENCIL
int inkcolors[6] = {WHITE, BLACK, DGRAY, LGRAY, LGRAY, DGRAY};
char *HelpMessage="You have a square grid; each square may contain a digit from 1 to the size of the grid. The grid is divided into blocks of varying shape and size, with arithmetic clues written in them. Your aim is to fully populate the grid with digits such that:\n\
  * Each row contains only one occurrence of each digit\n\
  * Each column contains only one occurrence of each digit\n\
  * The digits in each block can be combined to form the number stated in the clue, using the arithmetic operation given in the clue. That is:\n\
     o An addition clue means that the sum of the digits in the block must be the given number. For example, '15+' means the contents of the block adds up to fifteen.\n\
     o A multiplication clue (e.g. '60*'), similarly, means that the product of the digits in the block must be the given number.\n\
     o A subtraction clue will always be written in a block of size two, and it means that one of the digits in the block is greater than the other by the given amount. For example, '2-' means that one of the digits in the block is 2 more than the other, or equivalently that one digit minus the other one is 2. The two digits could be either way round, though.\n\
     o A division clue (e.g. '3/'), similarly, is always in a block of size two and means that one digit divided by the other is equal to the given amount.\n\n\
Keen controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move a highlight around the grid, and press OK to select a digit to enter it in the highlighted square.";
#endif

#ifdef LIGHTUP_GAME
// COL_BACKGROUND,COL_GRID,COL_BLACK,
// COL_LIGHT,			       /* white */
// COL_LIT,			       /* yellow */
// COL_ERROR,			       /* red */
// COL_CURSOR,
int inkcolors[7] = {WHITE, DGRAY, BLACK, WHITE, LGRAY, DGRAY, BLACK};
char *HelpMessage="You have a grid of squares. Some are filled in black; some of the black squares are numbered. Your aim is to 'light up' all the empty squares by placing light bulbs in some of them.\n\
Each light bulb illuminates the square it is on, plus all squares in line with it horizontally or vertically unless a black square is blocking the way.\n\
To win the game, you must satisfy the following conditions:\n\
  * All non-black squares are lit.\n\
  * No light is lit by another light.\n\
  * All numbered black squares have exactly that number of lights adjacent to them (in the four squares above, below, and to the side).\n\
Non-numbered black squares may have any number of lights adjacent to them.\n\n\
Light Up controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid.\n\
Press OK in a non-black square to toggle the presence of a light in that square. Press NEXTPAGE (or PLUS) key in a non-black square to mark there to aid solving; it can be used to highlight squares that cannot be lit, for example.";
#endif

#ifdef MAGNETS_GAME
// COL_BACKGROUND, COL_HIGHLIGHT, COL_LOWLIGHT,
// COL_TEXT, COL_ERROR, COL_CURSOR,
// COL_NEUTRAL, COL_NEGATIVE, COL_POSITIVE, COL_NOT
int inkcolors[10] = {WHITE, LGRAY, DGRAY, BLACK, DOTTED, LGRAY, DOTTED|LGRAY, BLACK, DGRAY, LGRAY};
char *HelpMessage="A rectangular grid has been filled with a mixture of magnets (that is, dominoes with one positive end and one negative end) and blank dominoes (that is, dominoes with two neutral poles). These dominoes are initially only seen in silhouette. Around the grid are placed a number of clues indicating the number of positive and negative poles contained in certain columns and rows.\n\
Your aim is to correctly place the magnets and blank dominoes such that all the clues are satisfied, with the additional constraint that no two similar magnetic poles may be orgothonally adjacent (since they repel). Neutral poles do not repel, and can be adjacacent to any other pole.\n\n\
Magnets controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move a cursor around the grid. Pressing the OK key will lay a domino with a positive pole at that position; pressing again reverses the polarity and then removes the domino. Using the NEXTPAGE (or PLUS) key allows placement of blank dominoes and cannot-be-blank hints.";
#endif

#ifdef MAP_GAME
// COL_BACKGROUND,COL_GRID,
// COL_0, COL_1, COL_2, COL_3,
// COL_ERROR, COL_ERRTEXT,
int inkcolors[8] = {WHITE, BLACK, DOTTED, DGRAY, DOTTED|LGRAY, LGRAY, BLACK, WHITE};
char *HelpMessage="You are given a map consisting of a number of regions. Your task is to colour each region with one of four colours, in such a way that no two regions sharing a boundary have the same colour. You are provided with some regions already coloured, sufficient to make the remainder of the solution unique.\n\
Only regions which share a length of border are required to be different colours. Two regions which meet at only one point (i.e. are diagonally separated) may be the same colour.\n\n\
Map controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the map: the colour of the cursor indicates the position of the colour you would drag (which is not obvious if you're on a region's boundary, since it depends on the direction from which you approached the boundary). Pressing the OK key starts a drag of that colour, which you control with the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys; pressing the OK key again finishes the drag. The NEXTPAGE (or PLUS) key can be used similarly to create a stippled region. Double-pressing the OK key (without moving the cursor) will clear the region: this is useful if you have filled the entire map in but need to correct the layout.";
#endif

#ifdef MINES_GAME
// COL_BACKGROUND, COL_BACKGROUND2,COL_1,COL_2,COL_3,COL_4,COL_5,COL_6,COL_7,COL_8,
// COL_MINE,COL_BANG,COL_CROSS,COL_FLAG,COL_FLAGBASE,COL_QUERY,
// COL_HIGHLIGHT, COL_LOWLIGHT,COL_WRONGNUMBER,COL_CURSOR
int inkcolors[20] = {WHITE, DOTTED|LGRAY, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, DGRAY, BLACK, LGRAY, BLACK, BLACK, DGRAY, LGRAY, BLACK, WHITE, DGRAY};
char *HelpMessage="You have a grid of covered squares, some of which contain mines, but you don't know which. Your job is to uncover every square which does not contain a mine. If you uncover a square containing a mine, you lose. If you uncover a square which does not contain a mine, you are told how many mines are contained within the eight surrounding squares.\n\n\
Mines controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the minefield. Pressing the OK key in a covered square uncovers it, and in an uncovered square will clear around it, pressing the NEXTPAGE (or PLUS) key in a covered square will place a flag.";
#endif

#ifdef NET_GAME
// COL_BACKGROUND,COL_LOCKED,COL_BORDER,COL_WIRE,COL_ENDPOINT,
// COL_POWERED,COL_BARRIER,
int inkcolors[7] = {WHITE, LGRAY, LGRAY, BLACK, LGRAY, LGRAY, BLACK};
char *HelpMessage="The computer prepares a network by connecting up the centres of squares in a grid, and then shuffles the network by rotating every tile randomly. Your job is to rotate it all back into place. The successful solution will be an entirely connected network, with no closed loops. As a visual aid, all tiles which are connected to the one in the middle are highlighted.\n\n\
Net controls:\n\
Select tile: \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys\n\
Rotate tile anticlockwise: OK key\n\
Rotate tile clockwise: NEXTPAGE (or PLUS) key\n\
Lock (or unlock) tile: PREVPAGE (or MINUS) key.";
#endif

#ifdef NETSLIDE_GAME
// COL_BACKGROUND,COL_FLASHING,COL_BORDER,COL_WIRE,COL_ENDPOINT,
// COL_POWERED,COL_BARRIER,COL_LOWLIGHT,COL_TEXT,
int inkcolors[9] = {WHITE, LGRAY, LGRAY, BLACK, LGRAY, LGRAY, DGRAY, DGRAY, BLACK};
char *HelpMessage="You have a Net grid, but instead of rotating tiles back into place you have to slide them into place by moving a whole row at a time.\n\n\
Netslide controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move the position indicator around the edge of the grid, and use the OK key to move the row/column in the direction indicated.";
#endif

#ifdef PATTERN_GAME
// COL_BACKGROUND,COL_EMPTY,COL_FULL,COL_TEXT,COL_UNKNOWN,COL_GRID,COL_CURSOR
int inkcolors[7] = {WHITE, WHITE, BLACK, BLACK, LGRAY, DGRAY, DGRAY};
char *HelpMessage="You have a grid of squares, which must all be filled in either black or white. Beside each row of the grid are listed the lengths of the runs of black squares on that row; above each column are listed the lengths of the runs of black squares in that column. Your aim is to fill in the entire grid black or white.\n\n\
Pattern controls:\n\
Move around the grid with the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys. Pressing the OK key will cycle the current cell through empty, then black, then white, then empty, and the NEXTPAGE (or PLUS) key does the same cycle in reverse.";
#endif

#ifdef PEGS_GAME
// COL_BACKGROUND,COL_HIGHLIGHT,COL_LOWLIGHT,COL_PEG,COL_CURSOR
int inkcolors[5] = {WHITE, LGRAY, DOTTED|DGRAY, BLACK, LGRAY};
char *HelpMessage="A number of pegs are placed in holes on a board. You can remove a peg by jumping an adjacent peg over it (horizontally or vertically) to a vacant hole on the other side. Your aim is to remove all but one of the pegs initially present.\n\n\
Pegs controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move a position indicator around the board. Pressing the OK key while over a peg, followed by a \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 key, will jump the peg in that direction (if that is a legal move).";
#endif

#ifdef RECT_GAME
// COL_BACKGROUND,COL_CORRECT,COL_LINE,COL_TEXT,COL_GRID,COL_DRAG,COL_DRAGERASE,COL_CURSOR
int inkcolors[8] = {WHITE, DGRAY, BLACK, BLACK, LGRAY, DGRAY, BLACK, LGRAY};
char *HelpMessage="You have a grid of squares, with numbers written in some (but not all) of the squares. Your task is to subdivide the grid into rectangles of various sizes, such that (a) every rectangle contains exactly one numbered square, and (b) the area of each rectangle is equal to the number written in its numbered square.\n\n\
Rectangles controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move the position indicator around the board. Pressing the OK key then allows you to use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to drag a rectangle out from that position, and pressing the OK key again completes the rectangle. Using the NEXTPAGE (or PLUS) key instead of the OK key allows you to erase the contents of a rectangle without affecting its edges.\n\
When a rectangle of the correct size is completed, it will be shaded.";
#endif

#ifdef SAMEGAME_GAME
// COL_BACKGROUND,
// COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9,
// COL_IMPOSSIBLE, COL_SEL, COL_HIGHLIGHT, COL_LOWLIGHT
int inkcolors[14] = {WHITE, BLACK, DGRAY, DOTTED|LGRAY, DOTTED|DGRAY, DOTTED, LGRAY, BLACK, BLACK, DGRAY, BLACK, WHITE, LGRAY, DGRAY};
char *HelpMessage="You have a grid of coloured squares, which you have to clear by highlighting contiguous regions of more than one coloured square; the larger the region you highlight, the more points you get (and the faster you clear the arena).\n\
If you clear the grid you win. If you end up with nothing but single squares (i.e., there are no more clickable regions left) you lose.\n\
Removing a region causes the rest of the grid to shuffle up: blocks that are suspended will fall down (first), and then empty columns are filled from the right.\n\n\
Same Game controls:\n\
The \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys move a cursor around the grid. Pressing the OK or NEXTPAGE (or PLUS) keys while the cursor is in an unselected region selects it; pressing OK or NEXTPAGE (or PLUS) again removes it.";
#endif

#ifdef SIGNPOST_GAME
// COL_BACKGROUND, COL_HIGHLIGHT, COL_LOWLIGHT,
// COL_GRID, COL_CURSOR, COL_ERROR, COL_DRAG_ORIGIN,
// COL_ARROW, COL_ARROW_BG_DIM,
// COL_NUMBER, COL_NUMBER_SET, COL_NUMBER_SET_MID,
// COL_B0,                             /* background colours */
// COL_M0 =   COL_B0 + 1*NBACKGROUNDS, /* mid arrow colours */
// COL_D0 =   COL_B0 + 2*NBACKGROUNDS, /* dim arrow colours */
// COL_X0 =   COL_B0 + 3*NBACKGROUNDS, /* dim arrow colours */
int inkcolors[76] = {WHITE, LGRAY, DGRAY,
    BLACK, DGRAY, DOTTED|BLACK, LGRAY, BLACK, DGRAY, BLACK, DGRAY, LGRAY,
    DOTTED|LGRAY, LGRAY, DGRAY, DOTTED|DGRAY, DOTTED, LGRAY, DGRAY, DOTTED|DGRAY, DOTTED, LGRAY, DGRAY, DOTTED|DGRAY, DOTTED, LGRAY, DGRAY, DOTTED|DGRAY,
    BLACK, DOTTED|LGRAY, DGRAY, DOTTED|DGRAY, DOTTED, LGRAY, DGRAY, DOTTED|DGRAY, DOTTED, LGRAY, DGRAY, DOTTED|DGRAY, DOTTED, LGRAY, DGRAY, DOTTED|DGRAY,
    DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY,
    DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY, DOTTED|LGRAY};

char *HelpMessage="You have a grid of squares; each square (except the last one) contains an arrow, and some squares also contain numbers. Your job is to connect the squares to form a continuous list of numbers starting at 1 and linked in the direction of the arrows - so the arrow inside the square with the number 1 will point to the square containing the number 2, which will point to the square containing the number 3, etc. Each square can be any distance away from the previous one, as long as it is somewhere in the direction of the arrow.\n\
By convention the first and last numbers are shown; one or more interim numbers may also appear at the beginning.\n\n\
Signpost controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid squares and lines. Pressing the return key when over a square starts a link operation, and pressing the return key again over a square will finish the link, if allowable. Pressing the space bar over a square will show the other squares pointing to it, and allow you to form a backward link, and pressing the space bar again cancels this.";
#endif

#ifdef SINGLES_GAME
// COL_BACKGROUND, COL_HIGHLIGHT, COL_LOWLIGHT,
// COL_BLACK, COL_WHITE, COL_BLACKNUM, COL_GRID,
// COL_CURSOR, COL_ERROR
int inkcolors[9] = {WHITE, LGRAY, DGRAY, BLACK, WHITE, DGRAY, DGRAY, DGRAY, LGRAY};
char *HelpMessage="You have a grid of white squares, all of which contain numbers. Your task is to colour some of the squares black (removing the number) so as to satisfy all of the following conditions:\n\
  * No number occurs more than once in any row or column.\n\
  * No black square is horizontally or vertically adjacent to any other black square.\n\
  * The remaining white squares must all form one contiguous region (connected by edges, not just touching at corners).\n\n\
Singles controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid. Pressing the OK or NEXTPAGE (or PLUS) keys will turn a square black or add a circle respectively, and pressing the key again will replace the number or remove the circle.";
#endif

#ifdef SIXTEEN_GAME
// same as fifteen
int inkcolors[4] = {WHITE, BLACK, LGRAY, DGRAY};
char *HelpMessage="You have a 4x4 square grid; all squares on the grid contain numbered squares. Your move is to shift an entire row left or right, or shift an entire column up or down; every time you do that, the tile you shift off the grid re-appears at the other end of the same row, in the space you just vacated. To win, arrange the tiles into numerical order (1,2,3,4 on the top row, 13,14,15,16 on the bottom). When you've done that, try playing on different sizes of grid.\n\n\
Sixteen controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move the position indicator around the edge of the grid, and use the OK key to move the row/column in the direction indicated.";
#endif

#ifdef SLANT_GAME
// COL_BACKGROUND,COL_GRID,COL_INK,COL_SLANT1,COL_SLANT2,COL_ERROR,COL_CURSOR,COL_LOWLIGHT
int inkcolors[8] = {WHITE, BLACK, BLACK, DGRAY, DGRAY, LGRAY, BLACK, LGRAY};
char *HelpMessage="You have a grid of squares. Your aim is to draw a diagonal line through each square, and choose which way each line slants so that the following conditions are met:\n\
  * The diagonal lines never form a loop.\n\
  * Any point with a circled number has precisely that many lines meeting at it. (Thus, a 4 is the centre of a cross shape, whereas a zero is the centre of a diamond shape - or rather, a partial diamond shape, because a zero can never appear in the middle of the grid because that would immediately cause a loop.)\n\n\
Slant controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid. Pressing the OK or NEXTPAGE (or PLUS) keys will place a \\ or a /, respectively, and will then cycle them as above.";
#endif

#ifdef SOLO_GAME
// COL_BACKGROUND,COL_XDIAGONALS,COL_GRID,COL_CLUE,COL_USER,
// COL_HIGHLIGHT,COL_ERROR,COL_PENCIL,COL_KILLER
int inkcolors[9] = {WHITE, DOTTED|LGRAY, BLACK, BLACK, DGRAY, LGRAY, LGRAY, DGRAY, BLACK};
char *HelpMessage="You have a square grid, which is divided into as many equally sized sub-blocks as the grid has rows. Each square must be filled in with a digit from 1 to the size of the grid, in such a way that\n\
  * every row contains only one occurrence of each digit\n\
  * every column contains only one occurrence of each digit\n\
  * every block contains only one occurrence of each digit.\n\
  * (optionally, by default off) each of the square's two main diagonals contains only one occurrence of each digit.\n\
You are given some of the numbers as clues; your aim is to place the rest of the numbers correctly.\n\n\
Solo controls:\n\
Move around the grid with the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys; press OK to select a digit - this will fill the square containing the cursor with that number, or selecting 0 will clear it.";
#endif

#ifdef TENTS_GAME
// COL_BACKGROUND,COL_GRID,COL_GRASS,COL_TREETRUNK,COL_TREELEAF,
// COL_TENT,COL_ERROR,COL_ERRTEXT,COL_ERRTRUNK,
int inkcolors[9] = {WHITE, BLACK, LGRAY, BLACK, DGRAY, DGRAY, WHITE, BLACK, DGRAY};
char *HelpMessage="You have a grid of squares, some of which contain trees. Your aim is to place tents in some of the remaining squares, in such a way that the following conditions are met:\n\
  * There are exactly as many tents as trees.\n\
  * The tents and trees can be matched up in such a way that each tent is directly adjacent (horizontally or vertically, but not diagonally) to its own tree. However, a tent may be adjacent to other trees as well as its own.\n\
  * No two tents are adjacent horizontally, vertically or diagonally.\n\
  * The number of tents in each row, and in each column, matches the numbers given round the sides of the grid.\n\n\
Tents controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move around the grid. Pressing the OK key over an empty square will place a tent, and pressing the NEXTPAGE (or PLUS) key over an empty square will colour it gray; either key will clear an occupied square.";
#endif

#ifdef TOWERS_GAME
// COL_BACKGROUND, COL_GRID, COL_USER,
// COL_HIGHLIGHT, COL_ERROR, COL_PENCIL
int inkcolors[6] = {WHITE, BLACK, DGRAY, LGRAY, LGRAY, DGRAY};
char *HelpMessage="You have a square grid. On each square of the grid you can build a tower, with its height ranging from 1 to the size of the grid. Around the edge of the grid are some numeric clues.\n\
Your task is to build a tower on every square, in such a way that:\n\
  * Each row contains every possible height of tower once\n\
  * Each column contains every possible height of tower once\n\
  * Each numeric clue describes the number of towers that can be seen if you look into the square from that direction, assuming that shorter towers are hidden behind taller ones. For example, in a 5?5 grid, a clue marked '5' indicates that the five tower heights must appear in increasing order (otherwise you would not be able to see all five towers), whereas a clue marked '1' indicates that the tallest tower (the one marked 5) must come first.\n\n\
Towers controls:\n\
Move around the grid with the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys; press OK to select a digit";
#endif

#ifdef TWIDDLE_GAME
// COL_BACKGROUND,COL_TEXT,COL_HIGHLIGHT,COL_HIGHLIGHT_GENTLE,COL_LOWLIGHT,
// COL_LOWLIGHT_GENTLE,COL_HIGHCURSOR, COL_LOWCURSOR
int inkcolors[8] = {WHITE, BLACK, LGRAY, DGRAY, DGRAY, LGRAY, WHITE, WHITE};
char *HelpMessage="Twiddle is a tile-rearrangement puzzle: you are given a grid of square tiles, each containing a number, and your aim is to arrange the numbers into ascending order.\n\
In basic Twiddle, your move is to rotate a square group of four tiles about their common centre. (Orientation is not significant in the basic puzzle, although you can select it.) On more advanced settings, you can rotate a larger square group of tiles.\n\n\
Twiddle controls:\n\
You can also move an outline square around the grid with the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys; the square is the size above (2x2 by default, or larger). Pressing the OK key or NEXTPAGE (or PLUS) key will rotate the current square anticlockwise or clockwise respectively.";
#endif

#ifdef UNEQUAL_GAME
// COL_BACKGROUND,COL_GRID,COL_TEXT,COL_GUESS,COL_ERROR,COL_PENCIL,
// COL_HIGHLIGHT, COL_LOWLIGHT,
int inkcolors[8] = {WHITE, BLACK, BLACK, DGRAY, LGRAY, LGRAY, LGRAY, DGRAY};
char *HelpMessage="You have a square grid; each square may contain a digit from 1 to the size of the grid, and some squares have clue signs between them. Your aim is to fully populate the grid with numbers such that:\n\
  * Each row contains only one occurrence of each digit\n\
  * Each column contains only one occurrence of each digit\n\
  * All the clue signs are satisfied.\n\
There are two modes for this game, 'Unequal' and 'Adjacent'.\n\
In 'Unequal' mode, the clue signs are greater-than symbols indicating one square's value is greater than its neighbour's. In this mode not all clues may be visible, particularly at higher difficulty levels.\n\
In 'Adjacent' mode, the clue signs are bars indicating one square's value is numerically adjacent (i.e. one higher or one lower) than its neighbour. In this mode all clues are always visible: absence of a bar thus means that a square's value is definitely not numerically adjacent to that neighbour's.\n\n\
Unequal controls:\n\
Use the \xE2\x86\x90\xE2\x86\x91\xE2\x86\x93\xE2\x86\x92 keys to move the mark around the grid. press OK to select a digit; selecting a 0 will clear a filled square.";
#endif


#ifdef UNTANGLE_GAME
// COL_BACKGROUND,COL_LINE,COL_CROSSEDLINE,COL_OUTLINE
// COL_POINT,COL_DRAGPOINT,COL_NEIGHBOUR,COL_FLASH1,COL_FLASH2
int inkcolors[8] = {WHITE, BLACK, LGRAY, BLACK, WHITE, LGRAY, BLACK, LGRAY};
char *HelpMessage="No help available";

#endif


#if defined (FILLING_GAME) || defined (SOLO_GAME) || defined (UNEQUAL_GAME) || defined (KEEN_GAME) || defined (TOWERS_GAME)
#define PBKEYB
#include "pbkeyb.c"
#endif

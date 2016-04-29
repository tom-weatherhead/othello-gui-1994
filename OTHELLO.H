// othello.h
// (C) Copyright 1994 by Tom Weatherhead


// Constants

#define BOARD_SIZE	8
#define NUM_VECTORS	8
#define MAX_CHANGES	4*(BOARD_SIZE-3)
#define BOARD_AREA	(BOARD_SIZE*BOARD_SIZE)
#define MAX_NUM_MOVES	BOARD_AREA-4
#define MIN_ROW		32
#define MIN_COL		32
#define SQUARE_WIDTH	32
#define SQUARE_HEIGHT	32
// NO_MARKER is a space
#define NO_MARKER	32

// INIT_MAX_EFFECT should be less than the sum of the values of
// the heuristic function for all board squares.
#define INIT_MAX_EFFECT	-9*BOARD_AREA

// Heuristic function
#define idx_weight(x)	((x==0||x==BOARD_SIZE-1)?BOARD_SIZE:1)
#define h_func(r,c)	(idx_weight(r)*idx_weight(c))

// Define a boolean type

#ifndef FALSE
#ifdef TRUE
#undef TRUE
#endif
typedef enum { FALSE, TRUE } bool;
#else
#ifndef TRUE
#define TRUE 1
#endif
typedef char bool;
#endif


// Other type definitions

typedef enum {
    PT_UNSET,
    PT_COMPUTER,
    PT_HUMAN
} player_type_type;

typedef struct {
    int r, c;
} vector_type;

typedef struct coord_struct {
    unsigned int r, c;
    // "tail" is a pointer to the "next" pointer of the last element
    // in the list.  This allows constant-time list splicing.
    struct coord_struct * next, ** tail;
} coord_type;

typedef struct {
    char ch[BOARD_SIZE][BOARD_SIZE];
} board_type;

class coord_heap_class {
  public: // protected?
    coord_type * heap;

    coord_heap_class( void );
    void enqueue( coord_type * );
    coord_type * dequeue( void );
    ~coord_heap_class( void );
};

class player_type {
  public:
    char marker;
    unsigned int count,		// The number of its markers on the board
      skill_level;
    player_type_type type;
    player_type * opponent;
    board_type * board;
    coord_heap_class * heap;

    void set_player_data( char );
};


// ******** Prototypes ********

unsigned int compute_effect( unsigned int, unsigned int, player_type *,
  vector_type *, unsigned int * );
int best_move( coord_type **, player_type * player,
  unsigned int, unsigned int, int, int );

// ******** End of othello.h ********

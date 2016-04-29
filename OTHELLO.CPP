// othello.cpp
// (C) Copyright 1992-1994 by Tom Weatherhead
//
// This program implements the game of othello, using the minimax algorithm
// (with alpha-beta pruning) to choose the computer's moves.
// The "verbose" option is useful for quickly debugging program semantics.
// (H)elp and (Q)uit are available at the command line.
// The best_move() function returns a linked list of best move records,
// to the depth specified by max_ply.  An application heap (manifested as
// a linked list) stores allocated but currently unused move records
// in an attempt to reduce the overhead of frequently callng malloc().

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include "othello.h"


// **** coord_heap_class member functions ****


coord_heap_class::coord_heap_class( void )
{
    heap = NULL;
}


// Add a list of move records to the application heap

void coord_heap_class::enqueue( coord_type * ptr )
{
    if( ptr == NULL ) return;

    *(ptr->tail) = heap;
#if 0
    // Not essential
    ptr->tail = &heap->tail; // Keeps heap linked to its tail
#endif
    heap = ptr;
} // enqueue()


// Get a blank move record, from the application heap or the real one

coord_type * coord_heap_class::dequeue( void )
{
    coord_type * rtn;

    if( heap == NULL ) {
	return( (coord_type *)malloc( sizeof( coord_type ) ) );
    } // if

    rtn = heap;
    heap = heap->next;
    return( rtn );
} // dequeue()


coord_heap_class::~coord_heap_class( void )
{
    coord_type * temp;

    for( ; heap != NULL; ) {
	temp = heap->next;
	free( heap );
	heap = temp;
    } // for
} // coord_heap_class::~coord_heap_class()


// **** player_type member functions ****

// Initialize a player's record.  Constructor?

void player_type::set_player_data( char marker2 )
{
    marker = marker2;
    type = ( marker == 'X' ) ? PT_HUMAN : PT_COMPUTER;
    skill_level = 3;
} // set_player_data()


// **** Non-member functions ****


// Using a heuristic, compute the gain resulting from a player's move.

unsigned int compute_effect( unsigned int row, unsigned int col,
  player_type * player, vector_type * changes, unsigned int * num_changedP )
{
    vector_type line_change[BOARD_SIZE-1];
    unsigned int row2, col2, num_changed = 0, i, heur_total=0, line_heur_total,
      len, drow, dcol, j;
    board_type * board = player->board;
    static vector_type vector[NUM_VECTORS] = { {-1,-1}, {-1,0}, {-1,1}, {0,-1},
                                               {0,1}, {1,-1}, {1,0}, {1,1} };

    for( i = 0; i < NUM_VECTORS; i++ ) {
	len = line_heur_total = 0;
	row2 = row;
	col2 = col;
	drow = vector[i].r;
	dcol = vector[i].c;

	for( ; ; ) {
	    row2 += drow;
	    col2 += dcol;

	    if( row2 >= BOARD_SIZE  ||  col2 >= BOARD_SIZE
	      ||  board->ch[row2][col2] == ' ' ) {
		len = 0;
		break;
	    } else if( board->ch[row2][col2] == player->marker ) break;

	    line_heur_total += h_func(row2,col2);
//	    assert( len < BOARD_SIZE - 1 );
	    line_change[len].r = row2;
	    line_change[len++].c = col2;
	} // for

	if( len == 0 ) continue;

	for( j = 0; j < len; j++ ) {
	    board->ch[line_change[j].r][line_change[j].c]
	      = player->marker;
	} // for

//	  assert( num_changed + len <= MAX_CHANGES );
	memcpy( &changes[num_changed], line_change, len * sizeof(*changes) );
	num_changed += len;
	heur_total += line_heur_total;
    } // for

    if( num_changed > 0 ) {
	heur_total += h_func(row,col);
	board->ch[row][col] = player->marker;	// Place marker now
        player->count += num_changed + 1;
        player->opponent->count -= num_changed;
    } // if

    if( num_changedP != NULL ) {
	*num_changedP = num_changed;
    } // if

    return( heur_total );
} // compute_effect()


// Compute the best-move chain (according to the minimax algorithm)

int best_move( coord_type ** rtn_chain, player_type * player,
  unsigned int ply, unsigned int max_ply, int prev_move_val, int best_sibling )
{
    bool done = FALSE;
    vector_type changes[MAX_CHANGES];
    int effect, max_effect = INIT_MAX_EFFECT;
    unsigned int num_changes, i, j, k, num_best_moves = 0, chosen_move;
    coord_type * best_moves[MAX_NUM_MOVES], * chain_ptr = NULL, * move_ptr;
    board_type * board = player->board;

    for( i = 0; i < BOARD_SIZE  &&  !done; i++ ) {

        for( j = 0; j < BOARD_SIZE  &&  !done; j++ ) {

            if( board->ch[i][j] != ' ' ) continue;

            // Make and record changes
            effect = compute_effect( i, j, player, changes, &num_changes );

            if( num_changes == 0 ) continue;

#if 0
	    if( verbose ) {
		printf( "Ply %d: %c placed at (%d,%d)\n", ply, player->marker,
		  i, j );
	    } // if
#endif

            if( ply < max_ply  &&
              player->count + player->opponent->count < BOARD_AREA ) {
                effect -= best_move( &chain_ptr, player->opponent,
		  ply + 1, max_ply, effect, max_effect );

		  // Insure that no moves are incorrectly ignored
//                  assert( effect > INIT_MAX_EFFECT );
            } // if

            if( effect > max_effect ) {

		for( k = 0; k < num_best_moves; k++ ) {
		    // Enqueue old best chains
		    player->heap->enqueue( best_moves[k] );
		} // for

		if( ply > 1  &&  prev_move_val - effect < best_sibling ) {
		    // Alpha-beta pruning is done here
#if 0
		    if( verbose ) {
			printf( "prune: %d - %d < %d\n", prev_move_val, effect,
			  best_sibling );
		    } // if
#endif
		    done = TRUE;
		} // if

                max_effect = effect;
                num_best_moves = 0;
	    } // if

            if( effect == max_effect ) {
		move_ptr = player->heap->dequeue();
		move_ptr->r = i;
		move_ptr->c = j;
		move_ptr->next = chain_ptr;
		move_ptr->tail = (chain_ptr != NULL) ? chain_ptr->tail
		  : &move_ptr->next;
                best_moves[num_best_moves++] = move_ptr;
	    } else {
		// Return chain to heap
		player->heap->enqueue( chain_ptr );
	    } // if

            // Remove marker and undo changes

            board->ch[i][j] = ' ';
            player->count -= num_changes + 1;
            player->opponent->count += num_changes;

            for( k = 0; k < num_changes; k++ ) {
		board->ch[changes[k].r][changes[k].c]
		  = player->opponent->marker;
            } // for
        } // for j
    } // for i

    if( rtn_chain == NULL ) {
    	// Do nothing; don't retrieve best-move chain
    } else if( num_best_moves == 0 ) {
//	printf( "Ply %d: no best move chosen\n", ply );
	*rtn_chain = NULL;
	max_effect = 0;
    } else {
        chosen_move = rand() % num_best_moves;
	*rtn_chain = best_moves[chosen_move];

	for( i = 0; i < num_best_moves; i++ ) { // Free other chains

	    if( i != chosen_move ) {
		player->heap->enqueue( best_moves[i] );
	    } // if
	} // if
#if 0
	if( verbose ) {
	    printf( "Chose move %d of %d\n", chosen_move, num_best_moves );
	    printf( "Ply %d: %c @ (%d,%d) => %d\n", ply, player->marker,
	      (*rtn_chain)->r, (*rtn_chain)->c, max_effect );
	} // if
#endif
    } // if

    return( max_effect );
} // best_move()


// ******** End of othello.cpp ********

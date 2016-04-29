// othelgui.cpp
// ObjectWindows-based UI for Othello       October 9, 1994
// (C) Copyright 1994 by Tom Weatherhead			

// ObjectWindows - (C) Copyright 1992 by Borland International

#define WIN31

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <string.h>
#include <owl.h>
#include <dialog.h>
#include <edit.h>
#include <checkbox.h>
#include <radiobut.h>
#include <groupbox.h>
#include "othello.h"
#include "othdispa.h"


#define LEVELEDITSIZE		3
#define O_USE_DIALOG		1


class TOptionsDlg
#ifdef O_USE_DIALOG
  : public TDialog
#else
  : public TWindow
#endif
  {
  private:
    void FillBuffers( void );

  public:
    char XLevelEdit[LEVELEDITSIZE];
    char OLevelEdit[LEVELEDITSIZE];
    player_type_type XPlayerType, OPlayerType;
#ifdef O_USE_DIALOG
    TCheckBox * XAutoBox, * OAutoBox;
#else
    TRadioButton * XHButton, * XCButton, * OHButton, * OCButton;
    TGroupBox * XGroupBox, * OGroupBox;
#endif

    TOptionsDlg(PTWindowsObject AParent, LPSTR name);
    virtual void HandleXAutoBox( RTMessage )
      = [ID_FIRST + ID_XCOMPBTN];
    virtual void HandleOAutoBox( RTMessage )
      = [ID_FIRST + ID_OCOMPBTN];
#ifndef O_USE_DIALOG
    virtual void HandleOK( RTMessage )
      = [ID_FIRST + IDOK];
    virtual void HandleCancel( RTMessage )
      = [ID_FIRST + IDCANCEL];
#endif
    virtual BOOL CanClose( void );
}; // TOptionsDlg


_CLASSDEF(TOthelloWindow)
class TOthelloWindow : public TWindow
{
  public:
    BOOL ButtonDown, PlayingGame, got_a_DC;
//    char FileName[MAXPATH];
    board_type board;
    coord_heap_class coord_heap;
    player_type X, O, * player;
    HDC BaseDC;		// Display Context

    TOthelloWindow(PTWindowsObject AParent, LPSTR ATitle);
    void DrawMarker( HDC, int, int );
    void DrawChanges( vector_type *, int );
    void GameMain( void );
    void SetupGame( void );
    virtual void CMFileExit( RTMessage )
      = [CM_FIRST + CM_FILEEXIT];
    virtual void CMGameOptions( RTMessage )
      = [CM_FIRST + CM_GAMEOPTIONS];
    virtual void CMGamePlay( RTMessage )
      = [CM_FIRST + CM_GAMEPLAY];
    virtual void CMAbout( RTMessage )
      = [CM_FIRST + CM_ABOUT];
    virtual void WMLButtonDown( RTMessage )
      = [WM_FIRST + WM_LBUTTONDOWN];
    virtual void WMLButtonUp( RTMessage )
      = [WM_FIRST + WM_LBUTTONUP];
    virtual void WMRButtonDown( RTMessage )
      = [WM_FIRST + WM_RBUTTONDOWN];
    virtual void Paint( HDC, PAINTSTRUCT& );
    ~TOthelloWindow( void );
}; // TOthelloWindow


class TOthelloApp : public TApplication
{
  public:
    TOthelloApp(LPSTR AName, HINSTANCE hInstance,
      HINSTANCE hPrevInstance,
      LPSTR lpCmdLine, int nCmdShow)
      : TApplication(AName, hInstance, hPrevInstance,
          lpCmdLine, nCmdShow) {};
    virtual void InitMainWindow( void );
}; // TOthelloApp


// Generic I/O routines

#if 0
int PrintMsg( int mb_type, char * msg )
{
    char * caption;
    int ack, mb_bits;

    switch( mb_type ) {
      case RSAMB_YESNO:
	caption = "Decision";
	ack = IDYES;
	mb_bits = MB_YESNO | MB_ICONQUESTION;
	break;

      case RSAMB_OKCANCEL:
	caption = "Information";
	ack = IDOK;
	mb_bits = MB_OK | MB_ICONINFORMATION;
	break;

      default: 	// RSAMB_OK, and anything else
	caption = "Information";
	ack = IDOK;
	mb_bits = MB_OKCANCEL | MB_ICONINFORMATION;
	break;
    }

    return( MessageBox( OthelloMainWin->HWindow, msg, caption,
      mb_bits ) == ack);
} // PrintMsg()
#endif


// **** TOptionsDlg Member Functions ****


TOptionsDlg::TOptionsDlg( PTWindowsObject AParent, LPSTR name )
#ifndef O_USE_DIALOG
  : TWindow( AParent, name )
#else
  : TDialog( AParent, name )
#endif
{
    TOthelloWindow * OthWin = (TOthelloWindow *)Parent;

    // Display current values
    sprintf( XLevelEdit, "%d", OthWin->X.skill_level );
    sprintf( OLevelEdit, "%d", OthWin->O.skill_level );
#ifdef O_USE_DIALOG
    XAutoBox = new TCheckBox( this, ID_XCOMPBTN, NULL );
    // Does this work?
    XAutoBox->SetCheck( (OthWin->X.type == PT_HUMAN)
      ? BF_UNCHECKED : BF_CHECKED );
    OAutoBox = new TCheckBox( this, ID_OCOMPBTN, NULL );
    OAutoBox->SetCheck( (OthWin->O.type == PT_HUMAN)
      ? BF_UNCHECKED : BF_CHECKED );
    new TEdit(this, ID_XLEVELEDIT, sizeof(XLevelEdit));
    new TEdit(this, ID_OLEVELEDIT, sizeof(OLevelEdit));
#else
    XGroupBox = new TGroupBox(this, ID_XPLAYERBOX, "Player X",
      8, 4, 53, 40 );
    XHButton = new TRadioButton(this, ID_XHUMANBTN, "Human",
      14, 14, 40, 12, XGroupBox );
    XCButton = new TRadioButton(this, ID_XCOMPBTN, "Computer",
      14, 28, 41, 12, XGroupBox );
    OGroupBox = new TGroupBox(this, ID_OPLAYERBOX, "Player O",
      75, 4, 53, 40 );
    OHButton = new TRadioButton(this, ID_OHUMANBTN, "Human",
      80, 14, 40, 12, OGroupBox );
    OCButton = new TRadioButton(this, ID_OCOMPBTN, "Computer",
      80, 28, 41, 12, OGroupBox );
    new TStatic( this, -1, "X Level", 7, 53, 29, 8, 7 );
    new TEdit(this, ID_XLEVELEDIT, XLevelEdit,
      44, 52, 16, 12, sizeof(XLevelEdit), FALSE );
    new TStatic( this, -1, "O Level", 75, 53, 29, 8, 7 );
    new TEdit(this, ID_OLEVELEDIT, OLevelEdit,
      109, 52, 16, 12, sizeof(OLevelEdit), FALSE );
    XHButton->SetCheck( BF_CHECKED );
    XCButton->SetCheck( BF_UNCHECKED );
    OHButton->SetCheck( BF_UNCHECKED );
    OCButton->SetCheck( BF_CHECKED );
#endif
    TransferBuffer = (void far *)NULL;

    // Refresh window; show loaded values (?)
} // TOptionsDlg::TOptionsDlg()


BOOL TOptionsDlg::CanClose( void )
{
#ifdef O_USE_DIALOG
    int XLevel, OLevel;
    TOthelloWindow * OthWin = (TOthelloWindow *)Parent;

//    if( cancel ) return( TRUE ); ???

    FillBuffers();
    XLevel = atoi( XLevelEdit );

    if( XLevel < 2  ||  XLevel > 10 ) {
	MessageBox(HWindow, "Invalid skill level for X",
	  "Input Error", MB_OK);
	return FALSE;
    }

    OLevel = atoi( OLevelEdit );

    if( OLevel < 2  ||  OLevel > 10 ) {
	MessageBox(HWindow, "Invalid skill level for O",
	  "Input Error", MB_OK);
	return FALSE;
    }

    // Don't copy value until verified as valid
    OthWin->X.skill_level = XLevel;
    OthWin->O.skill_level = OLevel;
    OthWin->X.type = XPlayerType;
    OthWin->O.type = OPlayerType;
#endif
    return( TRUE );
} // TOptionsDlg::CanClose()


// Retrieve data

void TOptionsDlg::FillBuffers( void )
{
    GetDlgItemText(HWindow, ID_XLEVELEDIT, XLevelEdit, LEVELEDITSIZE);
    GetDlgItemText(HWindow, ID_OLEVELEDIT, OLevelEdit, LEVELEDITSIZE);
    // Get Radio Buttons too
#ifdef O_USE_DIALOG
    XPlayerType = ( XAutoBox->GetCheck() == BF_UNCHECKED ) ? PT_HUMAN : PT_COMPUTER;
    OPlayerType = ( OAutoBox->GetCheck() == BF_UNCHECKED ) ? PT_HUMAN : PT_COMPUTER;
#else
    XPlayerType = ( XHButton->GetCheck() == BF_CHECKED ) ? PT_HUMAN : PT_COMPUTER;
    OPlayerType = ( OHButton->GetCheck() == BF_CHECKED ) ? PT_HUMAN : PT_COMPUTER;
#endif
} // TOptionsDlg::FillBuffers()


void TOptionsDlg::HandleXAutoBox( RTMessage )
{
    XAutoBox->Toggle();
} // TOptionsDlg::HandleXAutoBox()


void TOptionsDlg::HandleOAutoBox( RTMessage )
{
    OAutoBox->Toggle();
} // TOptionsDlg::HandleOAutoBox()


#ifndef O_USE_DIALOG
void TOptionsDlg::HandleOK( RTMessage )
{
    CloseWindow();
} // TOptionsDlg::HandleOK()


void TOptionsDlg::HandleCancel( RTMessage )
{
    CloseWindow();
} // TOptionsDlg::HandleCancel()
#endif


// **** TOthelloWindow Member Functions ****


TOthelloWindow::TOthelloWindow(PTWindowsObject AParent, LPSTR ATitle)
  : TWindow(AParent, ATitle)
{
    int i;

    Attr.X = Attr.Y = 0;
    Attr.W = 430;
    Attr.H = 352;
    AssignMenu( "COMMANDS" );

    for( i = 0; i < BOARD_SIZE; i++ ) {
	memset( board.ch[i], NO_MARKER, BOARD_SIZE );
    }

    ButtonDown = FALSE;
    PlayingGame = FALSE;
#if 1
    got_a_DC = FALSE;
#else
    // This causes grid to be drawn outside of window (?)
    got_a_DC = TRUE;
    BaseDC = GetDC( HWindow );
#endif
    X.opponent = &O;
    O.opponent = &X;
    X.board = O.board = &board;
    X.heap = O.heap = &coord_heap;
    X.set_player_data( 'X' );
    O.set_player_data( 'O' );
//    SetupGame();
} // TOthelloWindow::TOthelloWindow()


void TOthelloWindow::CMGameOptions( RTMessage )
{
#ifndef O_USE_DIALOG
    TOptionsDlg * OptWin = (TOptionsDlg *)(
      GetApplication()->MakeWindow(
	new TOptionsDlg( this, "Game Options" ) ) );

    // Disable parent window and show child window
    SetFocus( OptWin->HWindow );
#else
    if( GetModule()->ExecDialog( new TOptionsDlg( this, "GAMEOPTIONS" )) != IDOK ) return;
#endif
} // TOthelloWindow::CMGameOptions()


void TOthelloWindow::CMAbout( RTMessage )
{
    GetModule()->ExecDialog( new TDialog( this, "ABOUT" ) );
} // TOthelloWindow::CMAbout()


void TOthelloWindow::WMLButtonDown( RTMessage Msg )
{
    char buf[20];
    // row, col must be unsigned
    unsigned int row, col, num_changes;
    vector_type changes[MAX_CHANGES];

    if( !PlayingGame  ||  ButtonDown ) return;

    ButtonDown = TRUE;

    if( !got_a_DC ) return;

    memset( buf, ' ', sizeof( buf ) );
    TextOut( BaseDC, 1, 1, buf, sizeof( buf ) );

    // Coordinate is Msg.LP.Lo, Msg.LP.Hi
    col = (Msg.LP.Lo - MIN_COL) / SQUARE_WIDTH;
    row = (Msg.LP.Hi - MIN_ROW) / SQUARE_HEIGHT;

    // Assumes row, col are unsigned

    if( col >= BOARD_SIZE  ||  row >= BOARD_SIZE ) return;

    // Compute effect of move

    if( board.ch[row][col] != NO_MARKER ) {
//    	sprintf( buf, "Occupied 0x%02x", (int)(board.ch[row][col]) );
//    	TextOut( BaseDC, 1, 1, buf, strlen( buf ) );
	return;
    }

    compute_effect( row, col, player, changes, &num_changes );

    if( num_changes == 0 ) return;

    // Display the changes and the new marker
    changes[num_changes].r = row;
    changes[num_changes++].c = col;
    DrawChanges( changes, num_changes );

    // Let the other player play
    player = player->opponent;
    GameMain();
} // TOthelloWindow::WMLButtonDown()


void TOthelloWindow::WMLButtonUp( RTMessage )
{
    if( !ButtonDown ) return;

    ButtonDown = FALSE;
} // TOthelloWindow::WMLButtonUp()


void TOthelloWindow::WMRButtonDown( RTMessage )
{
    // Will cause WM_PAINT message to be sent
    InvalidateRect( HWindow, NULL, TRUE );
} // TOthelloWindow::WMRButtonDown()


void TOthelloWindow::Paint( HDC DC, PAINTSTRUCT& )
{
    int i, j;

    // Draw grid, 9 vert. lines, 9 horiz.

    for( i = 0; i <= BOARD_SIZE; i++ ) {
	// From (MIN_COL,MIN_ROW+i*SQUARE_HEIGHT)
	// 	to (MIN_COL+8*SQUARE_WIDTH,MIN_ROW+i*SQUARE_HEIGHT)
	MoveTo( DC, MIN_COL, MIN_ROW+i*SQUARE_HEIGHT );
	LineTo( DC, MIN_COL+8*SQUARE_WIDTH, MIN_ROW+i*SQUARE_HEIGHT );
	// From (MIN_COL+i*SQUARE_WIDTH,MIN_ROW)
	// 	to (MIN_COL+i*SQUARE_WIDTH,MIN_ROW+8*SQUARE_HEIGHT)
	MoveTo( DC, MIN_COL+i*SQUARE_WIDTH, MIN_ROW );
	LineTo( DC, MIN_COL+i*SQUARE_WIDTH, MIN_ROW+8*SQUARE_HEIGHT );
    } // for

    for( i = 0; i < BOARD_SIZE; i++ ) {

	for( j = 0; j < BOARD_SIZE; j++ ) {

	    if( board.ch[i][j] == NO_MARKER ) continue;

            DrawMarker( DC, j, i );
	} // for
    } // for
} // TOthelloWindow::Paint()


void TOthelloWindow::CMFileExit( RTMessage )
{
    CloseWindow();
} // TOthelloWindow::CMFileExit()


void TOthelloWindow::CMGamePlay( RTMessage )
{
    PlayingGame = TRUE;
    SetupGame();
} // TOthelloWindow::CMGamePlay()


void TOthelloWindow::DrawChanges( vector_type * changes, int num_changes )
{
    int i;

    for( i = 0; i < num_changes; i++ ) {
        // Delete the next line?
	board.ch[changes[i].r][changes[i].c] = player->marker;
	DrawMarker( BaseDC, changes[i].c, changes[i].r );
    } // for
} // TOthelloWindow::DrawChanges()


void TOthelloWindow::DrawMarker( HDC DCparam, int x, int y )
{
    char str[2];

    if( !got_a_DC ) return;

    str[0] = board.ch[y][x];
    str[1] = '\0';
    TextOut( DCparam, x * SQUARE_WIDTH + MIN_COL + 10,
     y * SQUARE_HEIGHT + MIN_ROW + 10, str, 1 );
} // TOthelloWindow::DrawMarker()

 
void TOthelloWindow::GameMain( void )
{
    char buf[20];
    bool can_go = TRUE, prev_can_go;
    unsigned int num_changes;
    coord_type * chain_ptr = NULL;
    vector_type changes[MAX_CHANGES];

    for( ; ; ) {
	sprintf( buf, "X %d; O %d", X.count, O.count );
	TextOut( BaseDC, 160, 1, buf, strlen( buf ) );
	// This game-over check catches the condition in either case
        // (ie. last player was (human|computer))

	if( player->count + player->opponent->count >= BOARD_AREA ) {
	    // Game over
            TextOut( BaseDC, 320, 17, "Game over", 9 );
	    PlayingGame = FALSE;
	    break;
	}

        // Check viability
	prev_can_go = can_go;
	can_go = ( best_move( &chain_ptr, player, 1, 1, 0, 0 )
	  > 0 ) ? TRUE : FALSE;

	if( !can_go ) {

	    if( !prev_can_go ) {
		PlayingGame = FALSE;	// Deadlock
	    	TextOut( BaseDC, 320, 17, "Deadlock", 8 );
		break;
	    } else {
		player = player->opponent;
		TextOut( BaseDC, 320, 17, "Can't go", 8 );
		continue;
	    }
	}

	if( player->type == PT_HUMAN ) break;

	// Play computer's move
	srand( time( NULL ) );
        best_move( &chain_ptr, player, 1, player->skill_level, 0, 0 );
//        printf( "Computer's move: %c placed at %d, %d\n",
//          player->marker, chain_ptr->r, chain_ptr->c );
	compute_effect( chain_ptr->r, chain_ptr->c, player, changes, &num_changes);
#if 0
	    printf( "Optimal chain:\n" );

	    for( move_ptr = chain_ptr, player_ptr = player; move_ptr != NULL;
	      move_ptr = move_ptr->next, player_ptr = player_ptr->opponent ) {
		printf( "%c: (%d,%d)\n", player_ptr->marker, move_ptr->r,
		  move_ptr->c );
	    } // for
#endif

	if( num_changes > 0 ) {
	    // Display changes
	    changes[num_changes].r = chain_ptr->r;
	    changes[num_changes++].c = chain_ptr->c;
	    DrawChanges( changes, num_changes );
	}

	coord_heap.enqueue( chain_ptr ); // Reuse the chain of move records
	player = player->opponent;
    } // for
} // TOthelloWindow::GameMain()

 
void TOthelloWindow::SetupGame( void )
{
    int i;

    for( i = 0; i < BOARD_SIZE; i++ ) {
	memset( board.ch[i], NO_MARKER, BOARD_SIZE );
    }

    if( !got_a_DC ) {
    	// Get a new DC
    	got_a_DC = TRUE;
    	BaseDC = GetDC( HWindow );
    }

    // Forces a call to WMPaint()
    InvalidateRect( HWindow, NULL, TRUE );

    X.count = O.count = 2;
    board.ch[4][3] = board.ch[3][4] = O.marker;
    board.ch[3][3] = board.ch[4][4] = X.marker;
    player = &X;
    PlayingGame = TRUE;
    GameMain();
} // TOthelloWindow::SetupGame()

 
TOthelloWindow::~TOthelloWindow( void )
{
    if( got_a_DC ) {
	ReleaseDC( HWindow, BaseDC );
    }
} // TOthelloWindow::~TOthelloWindow()


// **** TOthelloApp Member Functions ****


void TOthelloApp::InitMainWindow( void )
{
    MainWindow = new TOthelloWindow(NULL, Name);
//    OthelloMainWin = (TOthelloWindow *)MainWindow;
} // TOthelloApp::InitMainWindow()


int PASCAL WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TOthelloApp MyApp("Othello", hInstance, hPrevInstance,
      	       	 lpCmdLine, nCmdShow);

    MyApp.Run();
    return( MyApp.Status );
} // WinMain()


// ******** end of othelgui.cpp ********
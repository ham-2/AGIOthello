#  Kevan Hong-Nhan Nguyen 71632979.  ICS 32 Lab sec 9.  Project #5.

import othello
import othello_models
import tkinter
import threading
import asyncio

# Default / Initial Game Settings
DEFAULT_ROWS = 8
DEFAULT_COLUMNS = 8
DEFAULT_FIRST_PLAYER = othello.BLACK
DEFAULT_TOP_LEFT_PLAYER = othello.WHITE

# GUI Constants
BACKGROUND_COLOR = othello_models.BACKGROUND_COLOR
GAME_HEIGHT = 352
GAME_WIDTH = 352

class OthelloGUI:
    def __init__(self):
        # Initial Game Settings
        self._rows = 8
        self._columns = 8
        self._first_player = 'B'
        self._top_left_player = 'W'

        # Create my othello gamestate here (drawn from the original othello game code)
        self._game_state = othello.OthelloGame()
        
        # Initialize all my widgets and window here
        self._root_window = tkinter.Tk()
        self._root_window.title('Topoki GUI')
        self._root_window.iconbitmap("icon.ico")
        self._root_window.configure(background = BACKGROUND_COLOR)
        self._board = othello_models.GameBoard(self._game_state, GAME_WIDTH, GAME_HEIGHT, self._root_window)
        self._score = othello_models.Score(self._game_state, self._root_window)
        self._engine_out = othello_models.EngineOutput(self._game_state, self._root_window)
        self._eval_bar = othello_models.EvalBar(self._root_window)


        # Bind my game board with these two events.
        self._board.get_board().bind('<Configure>', self._on_board_resized)
        self._board.get_board().bind('<Button-1>', self._on_board_clicked)        


        # Create our menu that can be accessed at the top of the GUI
        self._menu_bar = tkinter.Menu(self._root_window)
        self._game_menu = tkinter.Menu(self._menu_bar, tearoff = 0)
        self._game_menu.add_command(label = 'New Game', command = self._new_game)
        self._game_menu.add_command(label = 'Game Settings', command = self._configure_game_settings)
        self._game_menu.add_separator()
        self._game_menu.add_command(label = 'Exit', command = self._root_window.destroy)
        self._menu_bar.add_cascade(label = 'Game', menu = self._game_menu)
        

        # Layout all the widgets here using grid layout
        self._root_window.config(menu = self._menu_bar)
        self._score.get_canvas().grid(row = 0, column = 0, columnspan = 2,
                                      padx = 0, pady = 10)
        self._eval_bar.get_canvas().grid(row = 1, column = 0,
                                         padx = 3, pady = 2)
        self._board.get_board().grid(row = 1, column = 1,
                                     padx = 3, pady = 2)
        self._engine_out.get_canvas().grid(row = 2, column = 0, columnspan = 2,
                               padx = 10, pady = 10)

        # Configure the root window's row/column weight (from the grid layout)
        self._root_window.rowconfigure(0, weight = 1)
        self._root_window.rowconfigure(1, weight = 1)
        self._root_window.rowconfigure(2, weight = 1)
        self._root_window.columnconfigure(0, weight = 1)
        self._root_window.columnconfigure(1, weight = 1)


    async def do_loop(self):
        while True:
            try:
                self._game_state.update()
                self._eval_bar.update(self._game_state._score, self._game_state._mate)
                self._board.update_bestmove(self._game_state._move)
                self._engine_out.update()
                self._root_window.update()
                await asyncio.sleep(0.05)
            except Exception as e:
                print(e)
                break

    async def start(self) -> None:
        ''' Runs the mainloop of the root window '''
        await self._game_state.start()
        await self.do_loop()


    def _configure_game_settings(self) -> None:
        ''' Pops out an options window to configure the game settings '''
        dialog = othello_models.OptionDialog(self._game_state._threads,
                                             self._game_state._hash)
        dialog.show()
        if dialog.was_ok_clicked():
            self._game_state.stop_analysis()
            self._game_state._threads = dialog.get_threads()
            self._game_state.set_option("Threads", dialog.get_threads())
            self._game_state._hash = dialog.get_hash()
            self._game_state.set_option("Hash", dialog.get_hash())
            self._game_state.resume_analysis()
            

    def _new_game(self) -> None:
        ''' Creates a new game with current _game_state settings '''
        self._game_state.new_game()
        self._board.new_game_settings(self._game_state)
        self._board.redraw_board()
        self._score.update_score(self._game_state)
        

    def _on_board_clicked(self, event: tkinter.Event) -> None:
        ''' Attempt to play a move on the board if it's valid '''
        move = self._convert_point_coord_to_move(event.x, event.y)
        row = move[0]
        col = move[1]
        try:
            self._game_state.move(row, col)
            self._board.update_game_state(self._game_state)
            self._board.redraw_board()
            self._score.update_score(self._game_state)

            if self._game_state.is_game_over():
                self._player_turn.display_winner(self._game_state.return_winner())
            else:
                self._player_turn.switch_turn(self._game_state)
        except:
            pass

    def _convert_point_coord_to_move(self, pointx: int, pointy: int) -> None:
        ''' Converts canvas point to a move that can be inputted in the othello game '''
        row = int(pointy // self._board.get_cell_height())
        if row == self._board.get_rows():
            row -= 1
        col = int(pointx // self._board.get_cell_width())
        if col == self._board.get_columns():
            col -= 1
        return (row, col)
        

    def _on_board_resized(self, event: tkinter.Event) -> None:
        ''' Called whenever the canvas is resized '''
        self._board.redraw_board()

    def __del__(self):
        self._root_window.quit()


if __name__ == '__main__':
    asyncio.run(OthelloGUI().start())




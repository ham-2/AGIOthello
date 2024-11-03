#  Kevan Hong-Nhan Nguyen 71632979.  ICS 32 Lab sec 9.  Project #5.

import othello
import othello_models
import tkinter
import threading
import asyncio

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
        self._mode = 0 # 0 : Analysis, 1: Self Play, 2: Play

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
        self._eval_bar_spacer = othello_models.Spacer(master = self._root_window,
                                                      width = 39, height = 352)
        self._eval_bar = othello_models.EvalBar(self._root_window)
        self._history = othello_models.History(self._game_state, self._root_window)
        self._button_frame = tkinter.Frame(self._root_window, background = BACKGROUND_COLOR)
        self._start_engine_button = othello_models.Button(self._button_frame)
        self._stop_engine_button = othello_models.Button(self._button_frame)
        self._button_spacer = othello_models.Spacer(master = self._button_frame,
                                             width = 78, height = 44)
        self._undo_button = othello_models.Button(self._button_frame)
        self._clock_white = othello_models.Clock(self._game_state, self._root_window, 'W')
        self._clock_black = othello_models.Clock(self._game_state, self._root_window, 'B')


        # Bind my game board with these two events.
        self._board.get_board().bind('<Configure>', self._on_board_resized)
        self._board.get_board().bind('<Button-1>', self._on_board_clicked)
        self._start_engine_button.get_canvas().bind('<Button-1>', self._start_engine) 
        self._stop_engine_button.get_canvas().bind('<Button-1>', self._stop_engine) 
        self._undo_button.get_canvas().bind('<Button-1>', self._undo_move) 

        # Create our menu that can be accessed at the top of the GUI
        self._menu_bar = tkinter.Menu(self._root_window)
        self._game_menu = tkinter.Menu(self._menu_bar, tearoff = 0)
        self._game_menu.add_command(label = 'New Game', command = self._new_game)
        self._game_menu.add_command(label = 'Play Topoki', command = self._configure_game_settings)
        self._game_menu.add_command(label = 'Analyze', command = self._start_analysis)
        self._game_menu.add_command(label = 'Game Settings', command = self._configure_play_settings)
        self._game_menu.add_separator()
        self._game_menu.add_command(label = 'Exit', command = self._root_window.destroy)
        self._menu_bar.add_cascade(label = 'Game', menu = self._game_menu)
        

        # Layout all the widgets here using grid layout
        self._root_window.config(menu = self._menu_bar)
        self._layout_default()
        self._layout_analysis()

        # Configure the root window's row/column weight (from the grid layout)
        self._root_window.rowconfigure(0, weight = 1)
        self._root_window.rowconfigure(1, weight = 1)
        self._root_window.rowconfigure(2, weight = 1)
        self._root_window.columnconfigure(0, weight = 1)
        self._root_window.columnconfigure(1, weight = 1)

        # Play Settings
        self._time = 300
        self._inc = 0
        self._user_side = 'B'
        self._show_bestmove = True

    async def do_loop(self):
        while True:
            try:
                self._game_state.update()
                self._eval_bar.update(self._game_state._score, self._game_state._mate)
                if self._show_bestmove: self._board.update_bestmove(self._game_state._move)
                self._engine_out.update()
                self._root_window.update()
                if self._game_state._playing:
                    self.callback_move()
                    wtime, btime = self._game_state.update_clock()
                    self._clock_white.update(wtime)
                    self._clock_black.update(btime)
                    if self._game_state.game_ended():
                        self._start_analysis()
                await asyncio.sleep(0.05)
            except Exception as e:
                print(e)
                break

    async def start(self) -> None:
        ''' Runs the mainloop of the root window '''
        await self._game_state.start()
        await self.do_loop()

    def _configure_play_settings(self) -> None:
        ''' Pops out an options window to configure the game settings '''
        dialog = othello_models.OptionDialog(self._game_state._threads,
                                             self._game_state._hash)
        dialog.show()
        resume = self._game_state._running
        if dialog.was_ok_clicked():
            self._game_state.stop_analysis()
            self._game_state._threads = dialog.get_threads()
            self._game_state.set_option("Threads", dialog.get_threads())
            self._game_state._hash = dialog.get_hash()
            self._game_state.set_option("Hash", dialog.get_hash())
            if resume: self._game_state.resume_analysis()
            
    def _configure_game_settings(self) -> None:
        ''' Pops out an options window to configure the game settings '''
        dialog = othello_models.PlayDialog(self._time, self._inc, 0)
        dialog.show()
        resume = self._game_state._running
        if dialog.get_side() != 3:
            side = 'B' if dialog.get_side() == 1 else 'W'
            self._game_state.set_game_info(dialog.get_time() * 1000,
                                           dialog.get_inc() * 1000,
                                           side, dialog.get_clock())
            self._start_play(dialog.get_bar(), side, dialog.get_clock())

    def _new_game(self) -> None:
        ''' Creates a new game with current _game_state settings '''
        self._game_state.stop_analysis()
        self._game_state.new_game()
        self._board.new_game_settings(self._game_state)
        self._board.redraw_board()
        self._score.update_score()
        self._history.update()
        self._game_state.resume_analysis()

    def _on_board_clicked(self, event: tkinter.Event) -> None:
        ''' Attempt to play a move on the board if it's valid '''
        if self._game_state._playing and self._game_state.turn != self._game_state._side:
            return
        move = self._convert_point_coord_to_move(event.x, event.y)
        row = move[0]
        col = move[1]
        try:
            self._game_state.move(row, col)
            self._history.update()
            self._board.update_game_state(self._game_state)
            self._board.redraw_board_cells(*self._game_state.get_lastmove())
            self._score.update_score()
        except Exception as e:
            print(e)
            pass

    def _convert_point_coord_to_move(self, pointx: int, pointy: int) -> None:
        ''' Converts canvas point to a move that can be inputted in the othello game '''
        row = int(pointy // self._board.get_cell_height())
        if row == 8: row -= 1
        col = int(pointx // self._board.get_cell_width())
        if col == 8: col -= 1
        return (row, col)
        
    def callback_move(self):
        self._board.redraw_board_cells(*self._game_state.get_lastmove())
        self._history.update()

    def _start_engine(self, *args):
        self._game_state.stop_analysis()
        self._game_state.resume_analysis()

    def _stop_engine(self, *args):
        self._game_state.stop_analysis()

    def _undo_move(self, *args):
        self._game_state.undo_move()
        self._history.update()
        #self._board.update_game_state(self._game_state)
        self._board.redraw_board_cells(*self._game_state.get_lastmove())
        self._score.update_score()

    def _on_board_resized(self, event: tkinter.Event) -> None:
        ''' Called whenever the canvas is resized '''
        self._board.redraw_board()

    def _layout_default(self):
        self._eval_bar_spacer.grid(row = 1, column = 0,
                                   padx = 0, pady = 2)
        self._board.get_board().grid(row = 1, column = 1,
                                     padx = 0, pady = 2)

    def _layout_play(self, showbar, side, limittime):
        self._score.get_canvas().grid_forget()
        self._score.hide_material()
        self._score.get_canvas().grid(row = 0, column = 0, columnspan = 2,
                                      padx = 5, pady = 3)
        self._button_frame.grid_forget()
        if not showbar: self._eval_bar.get_canvas().grid_forget()
        else: 
            self._eval_bar.get_canvas().grid(row = 1, column = 0,
                                             padx = 0, pady = 2)
        self._history.resize(True)
        self._history.get_canvas().grid(row = 1, column = 2, rowspan = 1,
                                        padx = 5, pady = 2, sticky = 'nw')
        self._engine_out.resize(True)
        self._engine_out.get_canvas().grid(row = 2, column = 2, columnspan = 1,
                               padx = 5, pady = 10)
        
        if side == 'B':
            self._clock_white.get_canvas().grid(row = 0, column = 1, columnspan = 1, 
                                                padx = 6, pady = 8, sticky = 'se')
            if limittime:
                self._clock_black.get_canvas().grid(row = 2, column = 0, columnspan = 2, 
                                                    padx = 6, pady = 8, sticky = 'ne')
        else:
            self._clock_black.get_canvas().grid(row = 0, column = 1, columnspan = 1, 
                                                padx = 6, pady = 8, sticky = 'se')
            if limittime:
                self._clock_white.get_canvas().grid(row = 2, column = 0, columnspan = 2, 
                                                    padx = 6, pady = 8, sticky = 'ne')

    def _layout_analysis(self):
        self._score.show_material()
        self._score.get_canvas().grid(row = 0, column = 0, columnspan = 2,
                                      padx = 5, pady = 3)
        self._button_frame.grid(row = 0, column = 2,
                                padx = 0, pady = 3, sticky = 'w')
        self._start_engine_button.get_canvas().grid(row = 0, column = 0)
        self._stop_engine_button.get_canvas().grid(row = 0, column = 1)
        self._button_spacer.grid(row = 0, column = 2)
        self._undo_button.get_canvas().grid(row = 0, column = 3)
        self._eval_bar.get_canvas().grid(row = 1, column = 0,
                                         padx = 0, pady = 2)
        self._history.resize(False)
        self._history.get_canvas().grid(row = 1, column = 2, rowspan = 2,
                                        padx = 5, pady = 2, sticky = 'nw')
        self._engine_out.resize(False)
        self._engine_out.get_canvas().grid(row = 2, column = 0, columnspan = 2,
                               padx = 5, pady = 10)
                               
        self._clock_white.get_canvas().grid_forget()
        self._clock_black.get_canvas().grid_forget()

    def _start_play(self, showbar, side, limittime):
        self._game_state.stop_analysis()
        self._layout_play(showbar, side, limittime)
        self._game_state.new_game()
        self._game_state._playing = True
        self._game_state._running = True
        self._show_bestmove = False
        self._game_state.set_clock()
        self._game_state.resume_analysis()
        
    def _start_analysis(self):
        self._game_state.stop_analysis()
        self._layout_analysis()
        self._game_state._playing = False
        self._show_bestmove = True

    def __del__(self):
        self._root_window.quit()


if __name__ == '__main__':
    asyncio.run(OthelloGUI().start())




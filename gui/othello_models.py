#  Kevan Hong-Nhan Nguyen 71632979.  ICS 32 Lab sec 9.  Project #5.

import othello
import tkinter
import random

# GUI / tkinter object constants
BACKGROUND_COLOR = '#2d2c29'
GAME_COLOR = '#667f49'
LIGHT_COLOR = '#ebecd0'
FONT_COLOR = '#d4d4bd'
BEST_COLOR = '#f9c24c'
LAST_COLOR = '#f5f682'
DISC_FONT = ('Calibri', 30)
DIALOG_FONT = ('Calibri', 14)
CLOCK_FONT = ('Calibri', 20, 'bold')
COORD_FONT = ('Calibri', 14, 'bold')
EVAL_FONT = ('Calibri', 10, 'bold')
ANALYSIS_FONT = ('Calibri', 14, 'bold')
HISTORY_FONT = ('Consolas', 12, 'bold')
PLAYERS = {othello.BLACK: 'Black', othello.WHITE: 'White'}
PIECE_COLOR = {othello.BLACK: '#555251', othello.WHITE: '#f6f6f6'}

class GameBoard:
    def __init__(self, game_state: othello.OthelloGame, game_width: float,
                 game_height: float, root_window) -> None:
        # Initialize the game board's settings here
        self._game_state = game_state
        self._cells = []
        self._board = tkinter.Canvas(master = root_window,
                                     width = game_width,
                                     height = game_height,
                                     background = GAME_COLOR)
        self._bestmove = '00'
        self._bestmove_lines = []

    def new_game_settings(self, game_state) -> None:
        ''' The game board's new game settings is now changed accordingly to
            the specified game state '''
        self._game_state = game_state

    def redraw_board(self) -> None:
        ''' Redraws the board '''
        self._board.delete(tkinter.ALL)
        self._redraw_lines()
        self._redraw_cells()
        self._redraw_coord()

    def redraw_board_cells(self, last_row, last_col):
        self._board.delete('c')
        self._redraw_cells(last_row, last_col)

    def _redraw_lines(self) -> None:
        ''' Redraws the board's lines '''
        row_multiplier = float(self._board.winfo_height()) / 8
        col_multiplier = float(self._board.winfo_width()) / 8
        
        # Draw the horizontal lines first
        for row in range(1, 8):
            self._board.create_line(
                0, row * row_multiplier,
                self.get_board_width(), row * row_multiplier,
                width = 3)

        # Draw the column lines next
        for col in range(1, 8):
            self._board.create_line(
                col * col_multiplier, 0,
                col * col_multiplier, self.get_board_height(),
                width = 3)

    def _redraw_cells(self, last_row = -1, last_col = -1) -> None:
        ''' Redraws all the occupied cells in the board '''
        if last_row != -1:
            self._board.create_rectangle(last_col * self.get_cell_width() + 1,
                                         last_row * self.get_cell_height() + 1,
                                         (last_col + 1) * self.get_cell_width() - 1,
                                         (last_row + 1) * self.get_cell_height() - 1,
                                         fill = LAST_COLOR,
                                         tag = 'c')
        for row in range(8):
            for col in range(8):
                if self._game_state.get_board()[row][col] != othello.NONE:
                    self._draw_cell(row, col) 
                
    def _draw_cell(self, row: int, col: int) -> None:
        ''' Draws the specified cell '''
        self._board.create_oval(col * self.get_cell_width(),
                                row * self.get_cell_height(),
                                (col + 1) * self.get_cell_width(),
                                (row + 1) * self.get_cell_height(),
                                fill = PIECE_COLOR[self._game_state.get_board()[row][col]],
                                width = 3, tag = 'c')                               

    def _redraw_coord(self):
        row_multiplier = float(self._board.winfo_height()) / 8
        col_multiplier = float(self._board.winfo_width()) / 8
        for row in range(8):
            self._board.create_text(
                10, row * row_multiplier + 32,
                text = chr(ord('1') + row),
                font = COORD_FONT)
        for col in range(8):
            self._board.create_text(
                col * col_multiplier + 35, 10,
                text = chr(ord('a') + col),
                font = COORD_FONT)

    def _redraw_bestmove(self, move):
        for _id in self._bestmove_lines :
            self._board.delete(_id)
            self._bestmove_lines.remove(_id)
        self._show_bestmove = self._game_state._running
        if move == '00':
            return
        row = ord(move[1]) - ord('1')
        col = ord(move[0]) - ord('a')
        row_multiplier = float(self._board.winfo_height()) / 8
        col_multiplier = float(self._board.winfo_width()) / 8
        self._bestmove_lines.append(self._board.create_line(
            (0.5 + col) * col_multiplier, (0.2 + row) * row_multiplier,
            (0.5 + col) * col_multiplier, (0.82 + row) * row_multiplier,
            width = 5, fill = BEST_COLOR))
        self._bestmove_lines.append(self._board.create_line(
            (0.2 + col) * col_multiplier, (0.5 + row) * row_multiplier,
            (0.82 + col) * col_multiplier, (0.5 + row) * row_multiplier,
            width = 5, fill = BEST_COLOR))

    def update_bestmove(self, move):
        self._redraw_bestmove(move)

    def update_game_state(self, game_state: othello.OthelloGame) -> None:
        ''' Updates our current _game_state to the specified one in the argument '''
        self._game_state = game_state

    def get_cell_width(self) -> float:
        ''' Returns a game cell's width '''
        return self.get_board_width() / 8

    def get_cell_height(self) -> float:
        ''' Returns a game cell's height '''
        return self.get_board_height() / 8

    def get_board_width(self) -> float:
        ''' Returns the board canvas's width '''
        return float(self._board.winfo_width())

    def get_board_height(self) -> float:
        ''' Returns the board canvas's height '''
        return float(self._board.winfo_height())

    def get_board(self) -> tkinter.Canvas:
        ''' Returns the game board '''
        return self._board


class Score:
    def __init__(self, game_state: othello.OthelloGame, root_window) -> None:
        ''' Initializes the score label '''
        self._g = game_state
        self._canvas = tkinter.Canvas(master = root_window,
                                    width = 420,
                                    height = 44,
                                    background = BACKGROUND_COLOR,
                                    bd = 0, highlightthickness = 0)
        self.show_material()

    def update_score(self) -> None:
        ''' Updates the score with the specified game state '''
        self._canvas.itemconfigure(self._tdisc,
                                  fill = PIECE_COLOR[self._g.get_turn()])
        self._canvas.itemconfigure(self._bdisc_cnt,
                                  text = "%2d" % (self._g.get_total_cells('B')))
        self._canvas.itemconfigure(self._wdisc_cnt,
                                  text = "%2d" % (self._g.get_total_cells('W')))

    def get_canvas(self):
        return self._canvas
        
    def show_material(self):
        self._canvas.delete(tkinter.ALL)
        self._tdisc = self._canvas.create_oval(5, 3,
                                 43, 41,
                                 fill = PIECE_COLOR[self._g.get_turn()],
                                 width = 3)        
        self._bdisc = self._canvas.create_oval(215, 3,
                                               253, 41,
                                               fill = PIECE_COLOR['B'],
                                               width = 3)
        self._wdisc = self._canvas.create_oval(320, 3,
                                               358, 41,
                                               fill = PIECE_COLOR['W'],
                                               width = 3)
        self._bdisc_cnt = self._canvas.create_text(265, 22, anchor = 'w',
                                                  text = '02', font = DISC_FONT, fill = PIECE_COLOR['W'])
        self._wdisc_cnt = self._canvas.create_text(370, 22, anchor = 'w',
                                                  text = '02', font = DISC_FONT, fill = PIECE_COLOR['W'])
        
        
    def hide_material(self):
        self._canvas.delete(tkinter.ALL)
        self._tdisc = self._canvas.create_oval(5, 3,
                                 43, 41,
                                 fill = PIECE_COLOR[self._g.get_turn()],
                                 width = 3)


# Dialog for when the user wants to change the game's settings
class OptionDialog:
    def __init__(self, curr_threads, curr_hash):
        self._dialog_window = tkinter.Toplevel(bg = BACKGROUND_COLOR)
        
        # Threads
        self._threads_frame = tkinter.Frame(master = self._dialog_window,
                                            bg = BACKGROUND_COLOR)
        self._threads_label = tkinter.Label(master = self._threads_frame,
                                        text = 'Threads:',
                                        font = DIALOG_FONT,
                                        bg = BACKGROUND_COLOR,
                                        fg = FONT_COLOR)
        self._threads_label.grid(row = 0, column = 0, sticky = tkinter.E,
                             padx = 3, pady = 3)
        self._threads = tkinter.IntVar()
        self._threads.set(curr_threads)
        self._threads_option_menu = tkinter.Spinbox(self._threads_frame,
                                                textvariable = self._threads,
                                                from_ = 1, to = 32,
                                                increment = 1,
                                                width = 3,
                                                bg = BACKGROUND_COLOR,
                                                fg = FONT_COLOR,
                                                buttonbackground = LIGHT_COLOR,
                                                bd = 0, highlightthickness = 0)
        self._threads_option_menu.grid(row = 0, column = 1, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._threads_frame.grid(row = 0, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)

        # Hash
        self._hash_frame = tkinter.Frame(master = self._dialog_window,
                                         bg = BACKGROUND_COLOR)
        self._hash_label = tkinter.Label(master = self._hash_frame,
                                        text = 'Hash:',
                                        font = DIALOG_FONT,
                                        bg = BACKGROUND_COLOR,
                                        fg = FONT_COLOR)
        self._hash_label.grid(row = 0, column = 0, sticky = tkinter.E,
                             padx = 5, pady = 5)
        self._hash = tkinter.IntVar()
        self._hash.set(curr_hash)
        self._hash_option_menu = tkinter.OptionMenu(self._hash_frame,
                                                    self._hash,
                                                    1, 2, 4, 8, 16, 32, 64)
        self._hash_option_menu.config(bg = BACKGROUND_COLOR,
                                      fg = FONT_COLOR,
                                      bd = 0, highlightthickness = 0)
        self._hash_option_menu.grid(row = 0, column = 1, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._hash_frame.grid(row = 1, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)


        # OK and Cancel Buttons
        self._button_frame = tkinter.Frame(master = self._dialog_window,
                                           bg = BACKGROUND_COLOR)
        self._button_frame.grid(row = 2, column = 0, sticky = tkinter.E,
                                padx = 20, pady = 3)
        
        self._ok_button = tkinter.Button(master = self._button_frame,
                                         text = 'OK',
                                         font = DIALOG_FONT,
                                         command = self._on_ok_button,
                                         pady = 0,
                                         bg = BACKGROUND_COLOR,
                                         fg = FONT_COLOR,
                                         bd = 0, highlightthickness = 0)
        self._ok_button.grid(row = 0, column = 0, padx = 3, pady = 0)

        self._cancel_button = tkinter.Button(master = self._button_frame,
                                             text = 'Cancel',
                                             font = DIALOG_FONT,
                                             command = self._on_cancel_button,
                                             pady = 0,
                                             bg = BACKGROUND_COLOR,
                                             fg = FONT_COLOR,
                                             bd = 0, highlightthickness = 0)
        self._cancel_button.grid(row = 0, column = 1, padx = 3, pady = 0)

        # Variable to determine what to do when the 'OK' button is clicked
        self._ok_clicked = False


    def show(self) -> None:
        self._dialog_window.grab_set()
        self._dialog_window.wait_window()

    def was_ok_clicked(self) -> bool:
        return self._ok_clicked

    def get_threads(self) -> int:
        return self._threads

    def get_hash(self) -> int:
        return self._hash

    # Functions assigned to button commands
    def _on_ok_button(self):
        self._ok_clicked = True
        self._threads = self._threads.get()
        self._hash = self._hash.get()
        self._dialog_window.destroy()

    def _on_cancel_button(self):
        self._dialog_window.destroy()

class RoundedRect:
    def __init__(self) -> None:
        self._canvas = self._canvas = tkinter.Canvas()

    def rr(self, x1, y1, x2, y2, radius=25) -> list:
        return [x1+radius, y1,
                x1+radius, y1,
                x2-radius, y1,
                x2-radius, y1,
                x2, y1,
                x2, y1+radius,
                x2, y1+radius,
                x2, y2-radius,
                x2, y2-radius,
                x2, y2,
                x2-radius, y2,
                x2-radius, y2,
                x1+radius, y2,
                x1+radius, y2,
                x1, y2,
                x1, y2-radius,
                x1, y2-radius,
                x1, y1+radius,
                x1, y1+radius,
                x1, y1]

    def get_canvas(self): return self._canvas


class EngineOutput(RoundedRect):
    def __init__(self, game_state: othello.OthelloGame, root_window) -> None:
        self._g = game_state
        self._canvas = tkinter.Canvas(master = root_window,
                                    width = 420,
                                    height = 58,
                                    background = BACKGROUND_COLOR,
                                    bd = 0, highlightthickness = 0)
        self.resize(False)

    def update(self):
        if self._g._playing:
            self._canvas.itemconfigure(self._depth,
                                  text = "Depth " + ('%02d' % self._g._depth) + \
                                      '@ {:.2f}'.format(self._g._nps) + " MNps\n" + \
                                     '{:.2f}'.format(self._g._nodes) + " MNodes")
        else:
            self._canvas.itemconfigure(self._depth,
                                  text = "Depth " + ('%02d' % self._g._depth))
            self._canvas.itemconfigure(self._nps,
                                  text = '{:.2f}'.format(self._g._nodes) + " MNodes @ " + \
                                     '{:.2f}'.format(self._g._nps) + " MNps")
            self._canvas.itemconfigure(self._pv,
                                  text = self._g._pv)

    def resize(self, play):
        if play:
            self._canvas.config(width = 210)
            self._canvas.delete(tkinter.ALL)
            self._box = self._canvas.create_polygon(self.rr(0, 0, 210, 58, 25), smooth = True)
            self._canvas.delete(self._pv)
            self._depth = self._canvas.create_text(10, 5, 
                                               text = '00',
                                               anchor = 'nw',
                                               font = ANALYSIS_FONT,
                                               fill = FONT_COLOR,
                                               justify = 'left')

        else:
            self._canvas.config(width = 420)
            self._canvas.delete(tkinter.ALL)
            self._box = self._canvas.create_polygon(self.rr(0, 0, 420, 58, 25), smooth = True)
            self._pv = self._canvas.create_text(10, 38,
                                            text = 'pv',
                                            anchor = 'w',
                                            font = ANALYSIS_FONT,
                                            fill = FONT_COLOR,
                                            justify = 'left')
            self._depth = self._canvas.create_text(10, 15, 
                                               text = '00',
                                               anchor = 'w',
                                               font = ANALYSIS_FONT,
                                               fill = FONT_COLOR,
                                               justify = 'left')
            self._nps = self._canvas.create_text(410, 15,
                                             text = '0 MNodes @ 1.0 MNps',
                                             anchor = 'e',
                                             font = ANALYSIS_FONT,
                                             fill = FONT_COLOR,
                                             justify = 'right')

class EvalBar:
    def __init__(self, root_window) -> None:
        self._prev_pos = 176
        self._canvas = tkinter.Canvas(master = root_window,
                                    width = 39,
                                    height = 352,
                                    background = PIECE_COLOR['B'])
        self.update(0, False)

    def get_canvas(self): return self._canvas

    def update(self, score, mate):
        # Eval to target pos
        if mate:
            t_pos = 360 if score < 0 else 0
        else:
            t_pos = -score * 352 / 40
            t_pos = min(150, t_pos)
            t_pos = max(-150, t_pos)
            t_pos += 177
        # Smooth
        t_pos = (self._prev_pos * 4 + t_pos) / 5
        self._prev_pos = t_pos

        self._canvas.delete(tkinter.ALL)
        self._canvas.create_rectangle(0, 0,
                                      43, t_pos,
                                      fill = PIECE_COLOR['W'])
        self._canvas.create_text(22, 10,
                                 text = 'M{0:+.0f}'.format(score) if mate \
                                     else '{0:+.2f}'.format(score),
                                 font = EVAL_FONT)

class History(RoundedRect):
    def __init__(self, game_state: othello.OthelloGame, root_window) -> None:
        self._g = game_state
        self._canvas = tkinter.Canvas(master = root_window,
                                      width = 210,
                                      height = 427,
                                      background = BACKGROUND_COLOR,
                                      bd = 0, highlightthickness = 0)
        self.resize(False)

    def update(self):
        history = self._g._history
        history_string = ''
        for i in range(len(history)):
            if (i % 2 == 0): history_string += '{:2d}'.format(i // 2 + 1) + '. '
            history_string += history[i] + ' '
            if (i % 4 == 1): history_string += ' '
            if (i % 4 == 3): history_string += '\n'

        history_string += '\n' + self._g._result

        self._canvas.itemconfigure(self._text,
                                  text = history_string)

    def resize(self, play):
        if play == True:
            self._canvas.configure(height = 362)
            self._canvas.delete(tkinter.ALL)
            self._box = self._canvas.create_polygon(self.rr(0, 0, 205, 362, 25), smooth = True)
            self._text = self._canvas.create_text(8, 15, 
                                        text = '',
                                        anchor = 'nw',
                                        font = HISTORY_FONT,
                                        fill = FONT_COLOR,
                                        justify = 'left')
        else:
            self._canvas.configure(height = 427)
            self._canvas.delete(tkinter.ALL)
            self._box = self._canvas.create_polygon(self.rr(0, 0, 205, 425, 25), smooth = True)
            self._text = self._canvas.create_text(8, 15, 
                                        text = '',
                                        anchor = 'nw',
                                        font = HISTORY_FONT,
                                        fill = FONT_COLOR,
                                        justify = 'left')
        self.update()


class Button(RoundedRect):
    def __init__(self, root_window) -> None:
        self._canvas = tkinter.Canvas(master = root_window,
                                      width = 44,
                                      height = 44,
                                      background = BACKGROUND_COLOR,
                                      bd = 0, highlightthickness = 0)
        self._canvas.create_polygon(self.rr(3, 3, 41, 41, 25), smooth = True,
                                   fill = BACKGROUND_COLOR,
                                   outline = 'white', width = 3)

class Spacer(tkinter.Canvas):
    def __init__(self, **kwargs):
        super().__init__(background = BACKGROUND_COLOR,
                      highlightthickness = 0,
                     **kwargs)


class PlayDialog:
    def __init__(self, curr_time, curr_inc, curr_bar):
        self._dialog_window = tkinter.Toplevel(bg = BACKGROUND_COLOR)
        
        # Time
        self._time_frame = tkinter.Frame(master = self._dialog_window,
                                            bg = BACKGROUND_COLOR)
        self._time_label = tkinter.Label(master = self._time_frame,
                                        text = 'Time(s):',
                                        font = DIALOG_FONT,
                                        bg = BACKGROUND_COLOR,
                                        fg = FONT_COLOR)
        self._time_label.grid(row = 0, column = 0, sticky = tkinter.E,
                             padx = 3, pady = 3)
        self._time = tkinter.IntVar()
        self._time.set(curr_time)
        self._time_option_menu = tkinter.Spinbox(self._time_frame,
                                                textvariable = self._time,
                                                from_ = 1, to = 999,
                                                increment = 1,
                                                width = 5,
                                                bg = BACKGROUND_COLOR,
                                                fg = FONT_COLOR,
                                                buttonbackground = LIGHT_COLOR,
                                                bd = 0, highlightthickness = 0)
        self._time_option_menu.grid(row = 0, column = 1, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._time_frame.grid(row = 0, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)

        # Increment
        self._inc_frame = tkinter.Frame(master = self._dialog_window,
                                            bg = BACKGROUND_COLOR)
        self._inc_label = tkinter.Label(master = self._inc_frame,
                                        text = 'Increment(s):',
                                        font = DIALOG_FONT,
                                        bg = BACKGROUND_COLOR,
                                        fg = FONT_COLOR)
        self._inc_label.grid(row = 0, column = 0, sticky = tkinter.E,
                             padx = 3, pady = 3)
        self._inc = tkinter.IntVar()
        self._inc.set(curr_inc)
        self._inc_option_menu = tkinter.Spinbox(self._inc_frame,
                                                textvariable = self._inc,
                                                from_ = 0, to = 999,
                                                increment = 1,
                                                width = 5,
                                                bg = BACKGROUND_COLOR,
                                                fg = FONT_COLOR,
                                                buttonbackground = LIGHT_COLOR,
                                                bd = 0, highlightthickness = 0)
        self._inc_option_menu.grid(row = 0, column = 1, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._inc_frame.grid(row = 1, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)

        # Time Control
        self._clock_frame = tkinter.Frame(master = self._dialog_window,
                                            bg = BACKGROUND_COLOR)
        self._clock = tkinter.IntVar()
        self._clock.set(True)
        self._clock_option_menu = tkinter.Checkbutton(self._clock_frame,
                                                    text = "Enable Clock for Player",
                                                    variable = self._clock,
                                                    bg = BACKGROUND_COLOR,
                                                    fg = FONT_COLOR,
                                                    bd = 0, highlightthickness = 0)
        self._clock_option_menu.grid(row = 0, column = 0, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._clock_frame.grid(row = 2, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)

        # Show Eval Bar
        self._bar_frame = tkinter.Frame(master = self._dialog_window,
                                            bg = BACKGROUND_COLOR)
        self._bar = tkinter.IntVar()
        self._bar.set(curr_bar)
        self._bar_option_menu = tkinter.Checkbutton(self._bar_frame,
                                                    text = "Show Eval Bar",
                                                    variable = self._bar,
                                                    bg = BACKGROUND_COLOR,
                                                    fg = FONT_COLOR,
                                                    bd = 0, highlightthickness = 0)
        self._bar_option_menu.grid(row = 0, column = 0, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._bar_frame.grid(row = 3, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)

        # Side / Cancel Buttons
        self._button_frame = tkinter.Frame(master = self._dialog_window,
                                           bg = BACKGROUND_COLOR)
        self._button_frame.grid(row = 4, column = 0, sticky = tkinter.E,
                                padx = 10, pady = 3)
        
        self._b_button = tkinter.Button(master = self._button_frame,
                                         text = 'Black',
                                         font = DIALOG_FONT,
                                         command = self._on_b_button,
                                         pady = 0,
                                         bg = BACKGROUND_COLOR,
                                         fg = FONT_COLOR,
                                         bd = 0, highlightthickness = 0)
        self._b_button.grid(row = 0, column = 0, padx = 3, pady = 0)

        self._w_button = tkinter.Button(master = self._button_frame,
                                         text = 'White',
                                         font = DIALOG_FONT,
                                         command = self._on_w_button,
                                         pady = 0,
                                         bg = BACKGROUND_COLOR,
                                         fg = FONT_COLOR,
                                         bd = 0, highlightthickness = 0)
        self._w_button.grid(row = 0, column = 1, padx = 3, pady = 0)

        self._r_button = tkinter.Button(master = self._button_frame,
                                         text = 'Random',
                                         font = DIALOG_FONT,
                                         command = self._on_r_button,
                                         pady = 0,
                                         bg = BACKGROUND_COLOR,
                                         fg = FONT_COLOR,
                                         bd = 0, highlightthickness = 0)
        self._r_button.grid(row = 0, column = 2, padx = 3, pady = 0)

        self._cancel_button = tkinter.Button(master = self._button_frame,
                                             text = 'Cancel',
                                             font = DIALOG_FONT,
                                             command = self._on_cancel_button,
                                             pady = 0,
                                             bg = BACKGROUND_COLOR,
                                             fg = FONT_COLOR,
                                             bd = 0, highlightthickness = 0)
        self._cancel_button.grid(row = 0, column = 3, padx = 3, pady = 0)

        # Variable to determine what to do when the 'OK' button is clicked
        self._clicked = 3


    def show(self) -> None:
        self._dialog_window.grab_set()
        self._dialog_window.wait_window()

    def get_side(self) -> int:
        return self._clicked

    def get_time(self) -> int:
        return self._time.get()

    def get_inc(self) -> int:
        return self._inc.get()

    def get_bar(self) -> bool:
        return bool(self._bar.get())

    def get_clock(self) -> bool:
        return bool(self._clock.get())

    # Functions assigned to button commands
    def _on_b_button(self):
        self._clicked = 1
        self._dialog_window.destroy()

    def _on_w_button(self):
        self._clicked = 2
        self._dialog_window.destroy()

    def _on_r_button(self):
        self._clicked = random.randrange(1, 2)
        self._dialog_window.destroy()

    def _on_cancel_button(self):
        self._dialog_window.destroy()
        
class Clock(RoundedRect):
    def __init__(self, game_state, root_window, color) -> None:
        self._g = game_state
        self._canvas = tkinter.Canvas(master = root_window,
                                      width = 96,
                                      height = 44,
                                      background = BACKGROUND_COLOR,
                                      bd = 0, highlightthickness = 0)
        fill_color = PIECE_COLOR['W'] if color == 'W' else PIECE_COLOR['B']                             
        text_color = PIECE_COLOR['B'] if color == 'W' else PIECE_COLOR['W']
        self._text_color = text_color
        self._canvas.create_polygon(self.rr(3, 3, 93, 41, 25), smooth = True,
                               fill = fill_color,
                               outline = text_color, width = 3)
        self._text = self._canvas.create_text(88, 22,
                                              text = "000:00",
                                              anchor = 'e',
                                              font = CLOCK_FONT, fill = text_color)
        self._time = 0

    def update(self, time):
        if time == self._time:
            self._canvas.delete('t')
        else:
            self._time = time
            min, sec = divmod(time, 60000)
            sec = int(sec / 1000)
            time_string = '{:3d}'.format(int(min)) + ':' + '{:02d}'.format(sec)
            self._canvas.itemconfigure(self._text,
                                      text = time_string)
            self._canvas.create_oval(10, 18, 18, 26,
                                     fill = self._text_color,
                                     tag = 't')
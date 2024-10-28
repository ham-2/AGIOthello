#  Kevan Hong-Nhan Nguyen 71632979.  ICS 32 Lab sec 9.  Project #5.

import othello
import tkinter

# GUI / tkinter object constants
BACKGROUND_COLOR = '#2d2c29'
GAME_COLOR = '#667f49'
LIGHT_COLOR = '#ebecd0'
FONT_COLOR = '#d4d4bd'
BEST_COLOR = '#f9c24c'
DISC_FONT = ('Calibri', 30)
DIALOG_FONT = ('Calibri', 14)
COORD_FONT = ('Calibri', 14, 'bold')
EVAL_FONT = ('Calibri', 10, 'bold')
ANALYSIS_FONT = ('Calibri', 14, 'bold')
PLAYERS = {othello.BLACK: 'Black', othello.WHITE: 'White'}
PIECE_COLOR = {othello.BLACK: '#555251', othello.WHITE: '#f6f6f6'}

class GameBoard:
    def __init__(self, game_state: othello.OthelloGame, game_width: float,
                 game_height: float, root_window) -> None:
        # Initialize the game board's settings here
        self._game_state = game_state
        self._rows = self._game_state.get_rows()
        self._cols = self._game_state.get_columns()
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
        self._rows = self._game_state.get_rows()
        self._cols = self._game_state.get_columns()

    def redraw_board(self) -> None:
        ''' Redraws the board '''
        self._board.delete(tkinter.ALL)
        self._redraw_lines()
        self._redraw_cells()
        self._redraw_coord()

    def _redraw_lines(self) -> None:
        ''' Redraws the board's lines '''
        row_multiplier = float(self._board.winfo_height()) / self._rows
        col_multiplier = float(self._board.winfo_width()) / self._cols
        
        # Draw the horizontal lines first
        for row in range(1, self._rows):
            self._board.create_line(
                0, row * row_multiplier,
                self.get_board_width(), row * row_multiplier,
                width = 3)

        # Draw the column lines next
        for col in range(1, self._cols):
            self._board.create_line(
                col * col_multiplier, 0,
                col * col_multiplier, self.get_board_height(),
                width = 3)

    def _redraw_cells(self) -> None:
        ''' Redraws all the occupied cells in the board '''
        for row in range(self._rows):
            for col in range(self._cols):
                if self._game_state.get_board()[row][col] != othello.NONE:
                    self._draw_cell(row, col)
                
    def _draw_cell(self, row: int, col: int) -> None:
        ''' Draws the specified cell '''
        self._board.create_oval(col * self.get_cell_width(),
                                row * self.get_cell_height(),
                                (col + 1) * self.get_cell_width(),
                                (row + 1) * self.get_cell_height(),
                                fill = PIECE_COLOR[self._game_state.get_board()[row][col]],
                                width = 3)                               

    def _redraw_coord(self):
        row_multiplier = float(self._board.winfo_height()) / self._rows
        col_multiplier = float(self._board.winfo_width()) / self._cols
        for row in range(8):
            self._board.create_text(
                10, row * row_multiplier + 35,
                text = chr(ord('1') + row),
                font = COORD_FONT)
        for col in range(8):
            self._board.create_text(
                col * col_multiplier + 38, 10,
                text = chr(ord('a') + col),
                font = COORD_FONT)

    def _redraw_bestmove(self, move):
        for _id in self._bestmove_lines :
            self._board.delete(_id)
            self._bestmove_lines.remove(_id)
        if move == '00':
            return
        row = ord(move[1]) - ord('1')
        col = ord(move[0]) - ord('a')
        row_multiplier = float(self._board.winfo_height()) / self._rows
        col_multiplier = float(self._board.winfo_width()) / self._cols
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
        return self.get_board_width() / self.get_columns()

    def get_cell_height(self) -> float:
        ''' Returns a game cell's height '''
        return self.get_board_height() / self.get_rows()

    def get_board_width(self) -> float:
        ''' Returns the board canvas's width '''
        return float(self._board.winfo_width())

    def get_board_height(self) -> float:
        ''' Returns the board canvas's height '''
        return float(self._board.winfo_height())

    def get_rows(self) -> int:
        ''' Returns the total number of rows in the board '''
        return self._rows

    def get_columns(self) -> int:
        ''' Returns the total number of rows in the board '''
        return self._cols

    def get_board(self) -> tkinter.Canvas:
        ''' Returns the game board '''
        return self._board


class Score:
    def __init__(self, game_state: othello.OthelloGame, root_window) -> None:
        ''' Initializes the score label '''
        self._canvas = tkinter.Canvas(master = root_window,
                                    width = 420,
                                    height = 44,
                                    background = BACKGROUND_COLOR,
                                    bd = 0, highlightthickness = 0)
        self._tdisc = self._canvas.create_oval(2, 3,
                                 40, 41,
                                 fill = PIECE_COLOR['B'],
                                 width = 3)
        self._canvas.create_oval(215, 3,
                                 253, 41,
                                 fill = PIECE_COLOR['B'],
                                 width = 3)
        self._canvas.create_oval(320, 3,
                                 358, 41,
                                 fill = PIECE_COLOR['W'],
                                 width = 3)
        self._bdisc = self._canvas.create_text(265, 22, anchor = 'w',
                                               text = '02', font = DISC_FONT, fill = PIECE_COLOR['W'])
        self._wdisc = self._canvas.create_text(370, 22, anchor = 'w',
                                               text = '02', font = DISC_FONT, fill = PIECE_COLOR['W'])

    def update_score(self, game_state: othello.OthelloGame) -> None:
        ''' Updates the score with the specified game state '''
        self._canvas.itemconfigure(self._tdisc,
                                  fill = PIECE_COLOR[game_state.get_turn()])
        self._canvas.itemconfigure(self._bdisc,
                                  text = "%2d" % (game_state.get_total_cells('B')))
        self._canvas.itemconfigure(self._wdisc,
                                  text = "%2d" % (game_state.get_total_cells('W')))

    def get_canvas(self) -> tkinter.Label:
        ''' Returns the score label '''
        return self._canvas

# Dialog for when the user wants to change the game's settings
class OptionDialog:
    def __init__(self, curr_threads, curr_hash):
        self._dialog_window = tkinter.Toplevel()

        self._threads = curr_threads
        self._hash = curr_hash
        
        # Threads
        self._threads_frame = tkinter.Frame(master = self._dialog_window)
        self._threads_label = tkinter.Label(master = self._threads_frame,
                                        text = 'Threads:',
                                        font = DIALOG_FONT)
        self._threads_label.grid(row = 0, column = 0, sticky = tkinter.E,
                             padx = 3, pady = 3)
        self._threads = tkinter.IntVar()
        self._threads.set(curr_threads)
        self._threads_option_menu = tkinter.Spinbox(self._threads_frame,
                                                textvariable = self._threads,
                                                from_ = 1, to = 32,
                                                increment = 1,
                                                width = 3)
        self._threads_option_menu.grid(row = 0, column = 1, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._threads_frame.grid(row = 0, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)

        # Hash
        self._hash_frame = tkinter.Frame(master = self._dialog_window)
        self._hash_label = tkinter.Label(master = self._hash_frame,
                                        text = 'Hash:',
                                        font = DIALOG_FONT)
        self._hash_label.grid(row = 0, column = 0, sticky = tkinter.E,
                             padx = 5, pady = 5)
        self._hash = tkinter.IntVar()
        self._hash.set(curr_threads)
        self._hash_option_menu = tkinter.OptionMenu(self._hash_frame,
                                                    self._hash,
                                                    1, 2, 4, 8, 16, 32, 64)
        self._hash_option_menu.grid(row = 0, column = 1, sticky = tkinter.W,
                                   padx = 3, pady = 3)
        self._hash_frame.grid(row = 1, column = 0, sticky = tkinter.W,
                             padx = 20, pady = 3)


        # OK and Cancel Buttons
        self._button_frame = tkinter.Frame(master = self._dialog_window)
        self._button_frame.grid(row = 2, column = 0, sticky = tkinter.E,
                                padx = 20, pady = 3)
        
        self._ok_button = tkinter.Button(master = self._button_frame,
                                         text = 'OK',
                                         font = DIALOG_FONT,
                                         command = self._on_ok_button,
                                         pady = 0)
        self._ok_button.grid(row = 0, column = 0, padx = 3, pady = 0)

        self._cancel_button = tkinter.Button(master = self._button_frame,
                                             text = 'Cancel',
                                             font = DIALOG_FONT,
                                             command = self._on_cancel_button,
                                             pady = 0)
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

class EngineOutput:
    def __init__(self, game_state: othello.OthelloGame, root_window) -> None:
        self._g = game_state
        self._canvas = tkinter.Canvas(master = root_window,
                                    width = 420,
                                    height = 58,
                                    background = BACKGROUND_COLOR,
                                    bd = 0, highlightthickness = 0)
        self._canvas.create_polygon(self.rr(0, 0, 420, 58, 25), smooth = True)
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
        self._pv = self._canvas.create_text(10, 38,
                                            text = 'pv',
                                            anchor = 'w',
                                            font = ANALYSIS_FONT,
                                            fill = FONT_COLOR,
                                            justify = 'left')

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

    def update(self):
        self._canvas.itemconfigure(self._depth,
                                  text = "Depth " + ('%02d' % self._g._depth))
        self._canvas.itemconfigure(self._nps,
                                  text = '{:.2f}'.format(self._g._nodes) + " MNodes @ " + \
                                     '{:.2f}'.format(self._g._nps) + " MNps")
        self._canvas.itemconfigure(self._pv,
                                  text = self._g._pv) 

    def get_canvas(self): return self._canvas

class EvalBar:
    def __init__(self, root_window) -> None:
        self._prev_pos = 176
        self._canvas = tkinter.Canvas(master = root_window,
                                    width = 35,
                                    height = 352,
                                    background = PIECE_COLOR['B'])
        self.update(0, False)

    def get_canvas(self): return self._canvas

    def update(self, score, mate):
        # Eval to target pos
        t_pos = -score * 352 / 28
        t_pos = min(170, t_pos)
        t_pos = max(-170, t_pos)
        t_pos += 176
        # Smooth
        t_pos = (self._prev_pos * 4 + t_pos) / 5
        self._prev_pos = t_pos

        self._canvas.delete(tkinter.ALL)
        self._canvas.create_rectangle(0, 0,
                                      40, t_pos,
                                      fill = PIECE_COLOR['W'])
        self._canvas.create_text(20, 10,
                                 text = 'M{0:+.0f}'.format(score) if mate \
                                     else '{0:+.2f}'.format(score),
                                 font = EVAL_FONT)
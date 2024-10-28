#  Kevan Hong-Nhan Nguyen 71632979.  ICS 32 Lab sec 9.  Project #5.

import asyncio
import subprocess
import sys
import queue

from typing import Type, TypeVar, Tuple, Union


# Game Constants
NONE = '.'
BLACK = 'B'
WHITE = 'W'

# An Exception that is raised every time an invalid move occurs
class InvalidMoveException(Exception):
    ''' Raised whenever an exception arises from an invalid move '''
    pass

class UCIProtocol(asyncio.SubprocessProtocol):
    def __init__(self) -> None:
        self.transport = None

        self.buffer = {
            1: bytearray(),  # stdout
            2: bytearray(),  # stderr
        }

        self.lines = queue.Queue();

    def connection_made(self, transport: asyncio.SubprocessTransport) -> None:
        self.transport = transport

    def send_line(self, line: str) -> None:
        stdin = self.transport.get_pipe_transport(0)
        try:
            stdin.write((line + '\n').encode("utf-8"))
            print('SENT: ' + line)
        except Exception as e:
            print(e)

    def pipe_data_received(self, fd: int, data: Union[bytes, str]) -> None:
        self.buffer[fd].extend(data)
        while b"\n" in self.buffer[fd]:
            line_bytes, self.buffer[fd] = self.buffer[fd].split(b"\n", 1)
            if line_bytes.endswith(b"\r"):
                line_bytes = line_bytes[:-1]
            line = line_bytes.decode("utf-8")
            print('RECEIVED: ' + line)
            self.lines.put(line)

    def pipe_connection_lost(self, fd: int, exc) -> None:
        print('closed' + fd)

    def process_exited(self, fd: int, exc) -> None:
        print('closed')

    def __del__(self):
        self.transport.kill()


# The Othello class that manages the game
class OthelloGame:
    '''
    Class that creates the Othello game and deals with all its game logic
    '''
    
    # Initialize the game through the __init__() function.
    # Within the function we initialize the game's board
    # by assigning it to a 2D list of strings through the
    # _new_game_board() function with its corresponding
    # arguments (to be received from the user in the user interface)
    def __init__(self):
        ''' Initialize all of the games settings and creates the board. '''
        self.rows = 8
        self.cols = 8
        self.current_board = self._new_game_board(8, 8, 'W')
        self.turn = 'B'

        # Engine Analysis
        self._threads = 1
        self._hash = 1
        self._depth = 0
        self._move = '00'
        self._score = +0.0
        self._pv = ''
        self._mate = False
        self._nodes = 0
        self._nps = 0.0

    async def start(self):
        (self.transport, self.uci) = await self._open_engine()
        self.new_game()

    def new_game(self):
        self.current_board = self._new_game_board(8, 8, 'W')
        self.turn = 'B'
        self._depth = 0
        self._move = '00'
        self._score = +0.0
        self._pv = ''
        self._mate = False
        self._send_engine('stop')
        self._send_engine('position startpos')
        self._send_engine('go infinite')

    def _new_game_board(self, rows: int, cols: int, top_left: str) -> [[str]]:
        ''' Creates the Othello Game board with specified dimensions. '''
        board =[]

        # Create an empty board
        for row in range(rows):
            board.append([])
            for col in range(cols):
                board[-1].append(NONE)

        # Initialize the 4 game pieces in the center
        board[rows // 2 - 1][cols // 2 - 1] = top_left
        board[rows // 2 - 1][cols // 2] = self._opposite_turn(top_left)
        board[rows // 2][cols // 2 - 1] = self._opposite_turn(top_left)
        board[rows // 2][cols // 2] = top_left
        
        return board

    async def _open_engine(self) -> Tuple[asyncio.SubprocessTransport, UCIProtocol]:
        return await asyncio.get_running_loop().subprocess_exec(
            UCIProtocol,
            program = "AGIOThello.exe",
            stdin = subprocess.PIPE,
            stdout = subprocess.PIPE)

    def _send_engine(self, msg):
        self.uci.send_line(msg)

    def _make_move(self, row, col):
        return chr(ord('a') + col) + chr(ord('1') + row)

    def stop_analysis(self):
        self._send_engine("stop")

    def resume_analysis(self):
        self._send_engine("go infinite")

    def set_option(self, name: str, value: int):
        self._send_engine("setoption name " + name + " value " + str(value))

    # This is the meat of the game logic. I define making a move through the
    # move() function, and within it are broken down helper functions that
    # make the code more reusable and the move() function more readable.
    def move(self, row: int, col: int) -> None:
        ''' Attempts to make a move at given row/col position.
            Current player/turn is the one that makes the move.
            If the player cannot make a move it raises an exception.
            If the player can make a move, the player finally plays
            the valid move and switches turn. '''

        # Check to see if the move is in a valid empty space
        # within the board's boundary
        self._require_valid_empty_space_to_move(row, col)


        # Retrieve a list of possible directions in which a valid move can occur.
        # (looks up to all 8 possible directions surrounding the move/cell)
        # The list only contains the directions where the opposite cell's color is
        # adjacent / touching the current cell. So it's not definite that the entire
        # list will return directions in which the cells in line of direction can be flipped.
        possible_directions = self._adjacent_opposite_color_directions(row, col, self.turn)


        # After having the list of possible directions, we begin keeping track of when a possible
        # valid move in a direction has been completed. The variable "next_turn" is used at the end
        # of this function to determine if the player switches turn, which only occurs if the
        # move is valid.
        #
        # The for loop looks through all of the possible directions, and if a direction is capable
        # of making a valid move/flip, then proceed into flipping the cells in that line of direction
        # and assign "next_turn" to the opposite turn to switch turns at the very end
        next_turn = self.turn
        for direction in possible_directions:
            if self._is_valid_directional_move(row, col, direction[0], direction[1], self.turn):
                next_turn = self._opposite_turn(self.turn)
            self._convert_adjacent_cells_in_direction(row, col, direction[0], direction[1], self.turn)


        # Here we decide if we can finally place down the current move and whether or not
        # we can switch turns. We decide to switch turns if the current player has made a valid move
        # AND the opposite player must have the option to be able to move in at least one empty cell
        # space. If the opposite player can't move in at least one empty cell space after the current
        # player has gone, we do not switch turns and the current player goes again for the second
        # time in a row.
        #
        # Ultimately, if the move is not valid, then raise an InvalidMoveException()
        if next_turn != self.turn:
            self._send_engine('stop')
            self.current_board[row][col] = self.turn
            if self.can_move(next_turn):
                self.switch_turn()
                self._send_engine('moves ' + self._make_move(row, col))
                self._send_engine('go infinite')
            else:
                self._send_engine('moves ' + self._make_move(row, col) + ' 00')
                self._send_engine('go infinite')
        else:
            raise InvalidMoveException()


    def _is_valid_directional_move(self, row: int, col: int, rowdelta: int, coldelta: int, turn: str) -> bool:
        ''' Given a move at specified row/col, checks in the given direction to see if
            a valid move can be made. Returns True if it can; False otherwise.
            Only supposed to be used in conjunction with _adjacent_opposite_color_directions()'''
        current_row = row + rowdelta
        current_col = col + coldelta

        last_cell_color = self._opposite_turn(turn)

        while True:
            # Immediately return false if the board reaches the end (b/c there's no blank
            # space for the cell to sandwich the other colored cell(s)
            if not self._is_valid_cell(current_row, current_col):
                break
            if self._cell_color(current_row, current_col) == NONE:
                break           
            if self._cell_color(current_row, current_col) == turn:
                last_cell_color = turn
                break

            current_row += rowdelta
            current_col += coldelta
            
        return last_cell_color == turn


    def _adjacent_opposite_color_directions(self, row: int, col: int, turn: str) -> [tuple]:
        ''' Looks up to a possible of 8 directions surrounding the given move. If any of the
            move's surrounding cells is the opposite color of the move itself, then record
            the direction it is in and store it in a list of tuples [(rowdelta, coldelta)].
            Return the list of the directions at the end. '''
        dir_list = []
        for rowdelta in range(-1, 2):
            for coldelta in range(-1, 2):
                if self._is_valid_cell(row+rowdelta, col + coldelta):
                    if self.current_board[row + rowdelta][col + coldelta] == self._opposite_turn(turn):
                        dir_list.append((rowdelta, coldelta))
        return dir_list
           

    def _convert_adjacent_cells_in_direction(self, row: int, col: int,
                                             rowdelta: int, coldelta: int, turn: str) -> None:
        ''' If it can, converts all the adjacent/contiguous cells on a turn in
            a given direction until it finally reaches the specified cell's original color '''
        if self._is_valid_directional_move(row, col, rowdelta, coldelta, turn):
            current_row = row + rowdelta
            current_col = col + coldelta
            
            while self._cell_color(current_row, current_col) == self._opposite_turn(turn):
                self._flip_cell(current_row, current_col)
                current_row += rowdelta
                current_col += coldelta


    # Functions to be used to determine if the game is over and what do when it is:
    #
    # is_game_over()
    # can_move()
    # return_winner()
    #
    def is_game_over(self) -> bool:
        ''' Looks through every empty cell and determines if there are
            any valid moves left. If not, returns True; otherwise returns False '''
        return self.can_move(BLACK) == False and self.can_move(WHITE) == False


    def can_move(self, turn: str) -> bool:
        ''' Looks at all the empty cells in the board and checks to
            see if the specified player can move in any of the cells.
            Returns True if it can move; False otherwise. '''
        for row in range(self.rows):
            for col in range(self.cols):
                if self.current_board[row][col] == NONE:
                    for direction in self._adjacent_opposite_color_directions(row, col, turn):
                        if self._is_valid_directional_move(row, col, direction[0], direction[1], turn):
                            return True
        return False

    def return_winner(self) -> str:
        ''' Returns the winner. ONLY to be called once the game is over.
            Returns None if the game is a TIE game.'''
        black_cells = self.get_total_cells(BLACK)
        white_cells = self.get_total_cells(WHITE)

        if black_cells == white_cells:
            return None
        else:
            if black_cells > white_cells:
                return BLACK
            else:
                return WHITE


    # Basic functions that perform simple tasks, ranging from retrieving
    # specific game data and switching turns:
    #
    # switch_turn()
    # get_rows()
    # get_columns()
    # get_turn()
    # get_total_cells()
    #
    def switch_turn(self) -> None:
        ''' Switches the player's turn from the current one to
            the other. Only to be called if the current player
            cannot move at all. '''
        self.turn = self._opposite_turn(self.turn)

    def get_board(self) -> [[str]]:
        ''' Returns the current game's 2D board '''
        return self.current_board

    def get_rows(self) -> int:
        ''' Returns the number of rows the game currently has '''
        return self.rows

    def get_columns(self) -> int:
        ''' Returns the number of columns the game currently has '''
        return self.cols

    def get_turn(self) -> str:
        ''' Returns the current game's turn '''
        return self.turn

    def get_total_cells(self, turn: str) -> int:
        ''' Returns the total cell count of the specified colored player '''
        total = 0
        for row in range(self.rows):
            for col in range(self.cols):
                if self.current_board[row][col] == turn:
                    total += 1
        return total


    # The rest of the functions are private functions only to be used within this module
    def _flip_cell(self, row: int, col: int) -> None:
        ''' Flips the specified cell over to the other color '''
        self.current_board[row][col] = self._opposite_turn(self.current_board[row][col])


    def _cell_color(self, row: int, col: int) -> str:
        ''' Determines the color/player of the specified cell '''
        return self.current_board[row][col]
        

    def _opposite_turn(self, turn: str) -> str:
        ''' Returns the player of the opposite player '''
        return {BLACK: WHITE, WHITE: BLACK}[turn]

    def _require_valid_empty_space_to_move(self, row: int, col: int) -> bool:
        ''' In order to move, the specified cell space must be within board boundaries
            AND the cell has to be empty '''
        
        if self._is_valid_cell(row, col) and self._cell_color(row, col) != NONE:
            raise InvalidMoveException()

    def _is_valid_cell(self, row: int, col: int) -> bool:
        ''' Returns True if the given cell move position is invalid due to
            position (out of bounds) '''
        return self._is_valid_row_number(row) and self._is_valid_col_number(col)

    def _is_valid_row_number(self, row: int) -> bool:
        ''' Returns True if the given row number is valid; False otherwise '''
        return 0 <= row < self.rows

    def _is_valid_col_number(self, col: int) -> bool:
        ''' Returns True if the given col number is valid; False otherwise '''
        return 0 <= col < self.cols

    def update(self):
        while not self.uci.lines.empty():
            out = self.uci.lines.get()
            tokens = out.split()
            if not tokens :
                continue

            if tokens[0] == 'info' :
                for i in range(len(tokens)):
                    if tokens[i] == 'depth':
                        self._depth = int(tokens[i + 1])
                        if self._depth > 99: self._depth = 99
                    elif tokens[i] == 'currmove':
                        self._move = tokens[i + 1]
                    elif tokens[i] == 'cd':
                        self._score = float(tokens[i + 1]) / 100
                        self._mate = False
                        if self.turn == 'W' :
                            self._score *= -1
                    elif tokens[i] == 'mate':
                        self._score = float(tokens[i + 1])
                        self._mate = True
                        if self.turn == 'W' :
                            self._score *= -1
                    elif tokens[i] == 'pv':
                        self._pv = ' '.join(tokens[i:])
                    elif tokens[i] == 'nodes':
                        self._nodes = float(tokens[i + 1]) / 1000000
                    elif tokens[i] == 'nps':
                        self._nps = float(tokens[i + 1]) / 1000000

            elif tokens[0] == 'bestmove':
                self._move = tokens[1]
# Topoki is Good

## Engine

Topoki is a NNUE othello engine.

### Compiling

### Usage
```
position startpos moves c4 e3
position fen O--OOOOX-OOOOOOXOOXXOOOXOOXOOOXXOOOOOOXX---OOOOX----O--X-------- b
```
position startpos: sets current position to the starting position. 
position fen: O(white), -(empty), X(black) from a1 to h8, followed by the side to move (b or w)
moves c4 e3: makes moves c4, e3 from the starting position or fen position.

```
go movetime 50
go infinite
```
go: starts search.

```
zero
rand 5
tune 32 1 16 1000 1e-11
```
zero: zeroes all weights.
rand: randomizes all weights. (5 bits in this case)
tune: starts learning.
Following arguments are: threads, self play search depth, start, number of games (divided by 1000), learning rate (1e-10 ~ 1e-11 recommended)

## GUI
```
python othello_gui.py
```
A simple gui is included. (from https://github.com/kevannguyen/Othello)
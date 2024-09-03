#include "alphabeta.h"

constexpr int NULLMOVE_MAX_PLY = 3;

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe, int step,
	int alpha, int beta) 
{

	if (*stop) { return EVAL_FAIL; }

	if (ply <= 0) { return eval(board); }

	else {
		// Move generation
		MoveList legal_moves;
		legal_moves.generate(board);

		Key root_key = board.get_key();

		// No legal moves
		if (legal_moves.list == legal_moves.end) {
			if (board.get_passed()) {
				return eval(board) ^ EVAL_END;
			}
			else { 
				TTEntry probe_m = {};
				Main_TT.probe(board.get_key(), &probe_m);

				Undo u;
				board.do_null_move(&u);
				int after_pass = -alpha_beta
				(
					board, stop,
					ply - 1, &probe_m, step,
					-beta, -alpha
				);
				board.undo_null_move();
				return after_pass;
			}
		}

		else {
			// Null Move Heuristic
			if (ply < NULLMOVE_MAX_PLY) {
				Undo u;
				board.do_null_move(&u);
				int null_value = -eval(board);
				board.undo_null_move();
				if (null_value > beta) { return null_value; }
			}

			Square s;
			Square nmove = probe->nmove;
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int reduction;
			bool can_skip = false;

			index = legal_moves.find_index(nmove);
			int i = 0;
			while ((i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				s = *(legal_moves.list + index);

				// Adjust alpha to calculate critical moves
				int lower_alpha = (5 << 23);

				// Do move
				Undo u;
				board.do_move(s, &u);
				TTEntry probe_m = {};

				// Probe Table and Search
				if (Main_TT.probe(board.get_key(), &probe_m) < 0)
				{ // Table Miss
					reduction = 0;
				}
				else
				{ // Table Hit
					comp_eval = -probe_m.eval;
					reduction = probe_m.depth + 1;
				}

				while (reduction < ply) {
					int lower_alpha_r = lower_alpha / (reduction + 1);

					if ((reduction > ply / 3) && (can_skip) &&
						(comp_eval < new_eval - lower_alpha_r))
					{
						break;
					}

					comp_eval = -alpha_beta(board, stop,
						reduction, &probe_m, step,
						-beta, -alpha + lower_alpha_r
					);

					reduction++;
				}
				
				if (comp_eval > new_eval) {
					new_eval = comp_eval;
					nmove = s;
					if (new_eval > alpha) { alpha = new_eval; }
					if (alpha > beta) {
						board.undo_move(s);
						break;
					}
				}

				board.undo_move(s);
				i++;
				index += step;
				can_skip = true;
			}

			if (!(*stop)) {
				Main_TT.register_entry(root_key, ply, new_eval, nmove);
			}

			return new_eval;
		}
	}

}
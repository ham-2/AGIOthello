#include "alphabeta.h"

constexpr int NULLMOVE_MAX_PLY = 3;

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe, int step,
	int alpha, int beta) 
{

	if (*stop) { return EVAL_FAIL; }

	// Move generation
	MoveList legal_moves;
	legal_moves.generate(board);

	Key root_key = board.get_key();

	// No legal moves
	if (legal_moves.list == legal_moves.end) {

		if (board.get_passed()) {
			int score = get_material_eval(board);

			Main_TT.register_entry(root_key, 64, score, GAME_END);

			return score;
		}

		else {
			Undo u;
			board.do_null_move(&u);

			TTEntry probe_m = {};
			Main_TT.probe(board.get_key(), &probe_m);

			int after_pass = -alpha_beta
			(
				board, stop,
				ply - 1, &probe_m, step,
				-beta, -alpha
			);
			board.undo_null_move();

			if (!(*stop)) {
				Main_TT.register_entry(root_key, ply, after_pass, NULL_MOVE);
			}

			return after_pass;
		}
	}

	// Legal moves
	else {

		if (ply <= 0) { return eval(board); }

		else {

			Square s;
			Square nmove = probe->nmove;
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int reduction;

			index = legal_moves.find_index(nmove);
			int i = 0;
			while ((i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				s = *(legal_moves.list + index);

				// Adjust alpha to calculate critical moves
				int lower_alpha = (8 << (EVAL_BITS - 6));

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

					if ((reduction > ply / 2) && (i > 0) &&
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
						board.undo_move();
						break;
					}
				}

				board.undo_move();
				i++;
				index += step;
			}

			if (!(*stop)) {
				Main_TT.register_entry(root_key, ply, new_eval, nmove);
			}

			return new_eval;
		}
	}

}
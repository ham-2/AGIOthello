#include "alphabeta.h"

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
		Undo u;
		board.do_null_move(&u);

		legal_moves.generate(board);
		if (legal_moves.list == legal_moves.end) {
			board.undo_null_move();
			int score = get_material_eval(board);
			probe->type = 0;
			Main_TT.register_entry(root_key, score, GAME_END, 0, 0);
			return score;
		}

		TTEntry probe_m = {};
		Main_TT.probe(board.get_key(), &probe_m);

		int after_pass = -alpha_beta
		(
			board, stop,
			ply - 1, &probe_m, step,
			-beta, -alpha
		);
		board.undo_null_move();

		probe->type = -probe_m.type;
		if (!(*stop)) {
			Main_TT.register_entry(root_key, after_pass, NULL_MOVE, ply, probe->type);
		}
		return after_pass;
	}

	// Legal moves
	else {

		if (ply <= 0) { 
			probe->type = 0;
			//if (is_mate(beta)) { return -EVAL_INIT - 1; }
			return eval(board);
		}

		else {

			Square s;
			Square nmove = probe->nmove;
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int alpha_i = alpha;
			int reduction;

			index = legal_moves.find_index(nmove);
			int i = 0;
			probe->type = 0;
			while ((i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				s = *(legal_moves.list + index);

				// Do move
				Undo u;
				board.do_move(s, &u);
				TTEntry probe_m;

				// Probe Table and Search
				Main_TT.probe(board.get_key(), &probe_m);
				if (is_miss(&probe_m, board.get_key()))
				{ // Table Miss
					reduction = 0;
				}
				else {
					comp_eval = -probe_m.eval;
					if (probe_m.depth >= ply - 1 &&
						comp_eval >= beta &&
						probe_m.type != -1) {
						new_eval = comp_eval;
						alpha = comp_eval;
						nmove = s;
						board.undo_move();
						probe->type = -1;
						break;
					} // Prune
					if (probe_m.depth >= ply - 1 &&
						comp_eval < new_eval &&
						probe_m.type != 1) {
						board.undo_move();
						i++;
						index += step;
						continue;
					} // Skip

					// Search from
					reduction = probe_m.nmove == GAME_END ? ply :
						min(probe_m.depth + 1, ply - 1);
				}

				while (reduction < ply) {
					int p_delta = 2048 / (1 + reduction);

					if ((probe_m.type != 1) &&
						(reduction > ply / 2) &&
						(reduction > 0) &&
						!is_mate(alpha) &&
						comp_eval < new_eval - p_delta) {
						break;
					}

					comp_eval = -alpha_beta(board, stop,
						reduction, &probe_m, step,
						-beta, -alpha
					);

					//if (comp_eval > alpha && probe_m.type == -1) { throw; }

					reduction++;
				}
				
				if (comp_eval > new_eval) {
					new_eval = comp_eval;
					nmove = s;
					if (new_eval > alpha) { 
						alpha = new_eval;
						if (alpha >= beta) {
							board.undo_move();
							probe->type = -1;
							break;
						}
					}
				}

				board.undo_move();
				i++;
				index += step;
			}

			if (alpha == alpha_i) { probe->type = 1; }

			if (!(*stop)) {
				Main_TT.register_entry(root_key, alpha, nmove, ply, probe->type);
			}

			return alpha;
		}
	}
}
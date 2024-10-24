#include "alphabeta.h"

int alpha_beta(SearchParams* sp, TTEntry* probe,
	int ply, int alpha, int beta) 
{
	Position* board = sp->board;
	atomic<bool>* stop = sp->stop;
	TT* table = sp->table;
	int step = sp->step;

	if (*stop) { return EVAL_FAIL; }

	// Move generation
	MoveList legal_moves;
	legal_moves.generate(*board);

	Key root_key = board->get_key();

	// No legal moves
	if (legal_moves.list == legal_moves.end) {
		board->pass();
		legal_moves.generate(*board);

		if (legal_moves.list == legal_moves.end) {
			board->pass();
			node_count++;
			int score = get_material_eval(*board);
			probe->type = 0;
			table->register_entry(root_key, score, GAME_END, 0, 0);
			return score;
		}

		TTEntry probe_m = {};
		table->probe(board->get_key(), &probe_m);

		int after_pass = -alpha_beta
		(
			sp, &probe_m,
			ply - 1, -beta, -alpha
		);
		board->pass();

		probe->type = -probe_m.type;
		if (!(*stop)) {
			table->register_entry(root_key, after_pass, NULL_MOVE, ply, probe->type);
		}
		return after_pass;
	}

	// Legal moves
	else {
		
		if (ply <= 0) { 
			//if (is_mate(alpha) || is_mate(beta)) {
			//	ply = 1;
			//}
			//else {
				probe->type = 0;
				int score = eval(*board);
				table->register_entry(root_key, score, SQ_END, 0, 0);
				return score;
			//}
		}

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
			Bitboard c;
			board->do_move(s, &c);
			TTEntry probe_m;

			// Probe Table and Search
			table->probe(board->get_key(), &probe_m);
			if (is_miss(&probe_m, board->get_key()))
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
					board->undo_move(s, &c);
					probe->type = -1;
					break;
				} // Prune
				if (probe_m.depth >= ply - 1 &&
					comp_eval < new_eval &&
					probe_m.type != 1) {
					board->undo_move(s, &c);
					i++;
					index += step;
					continue;
				} // Skip

				// Search from
				reduction = probe_m.nmove == GAME_END ? ply :
					min(probe_m.depth + 1, ply - 1);
			}

			while (reduction < ply) {
				int p_delta = (8 << (EVAL_BITS - 6)) / (1 + reduction);

				if ((probe_m.type != 1) &&
					(reduction > ply / 3) &&
					!is_mate(alpha) &&
					comp_eval < new_eval - p_delta) {
					break;
				}

				comp_eval = -alpha_beta(sp, &probe_m,
					reduction, -beta, -alpha);

				//if (comp_eval > alpha && probe_m.type == -1) { throw; }

				reduction++;
			}
				
			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
				if (new_eval > alpha) { 
					alpha = new_eval;
					if (alpha >= beta) {
						board->undo_move(s, &c);
						probe->type = -1;
						break;
					}
				}
			}

			board->undo_move(s, &c);
			i++;
			index += step;
		}

		if (alpha == alpha_i) { probe->type = 1; }

		if (!(*stop)) {
			table->register_entry(root_key, alpha, nmove, ply, probe->type);
		}

		return alpha;
	}
}
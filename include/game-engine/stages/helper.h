#ifndef STAGES_HELPER_H
#define STAGES_HELPER_H

#include "game-engine/stages/common.h"
#include "game-engine/board.h"
#include "game-engine/board-objects/minion.h"

namespace GameEngine {

class StageHelper
{
public:
	typedef Move::PlayMinionData PlayMinionData;

public: // return true if game state changed (e.g., win/loss)
	static bool PlayerDrawCard(Board &board, Player & player);

	static void GetBoardMoves(
		Board const & board, Player const& player, NextMoveGetter & next_moves, bool & all_cards_determined);
	static void GetBoardMoves_PlayMinion(
		Board const& board, Player const& player, Hand::Locator hand_card, Card const& playing_card, NextMoveGetter &next_move_getter);
	static void GetBoardMoves_EquipWeapon(
		Board const& board, Player const& player, Hand::Locator hand_card, Card const& playing_card, NextMoveGetter &next_move_getter);
	static void GetBoardMoves_Attack(
		Board const& board, Player const& player, NextMoveGetter &next_move_getter);
	static void GetGoodBoardMove(
		unsigned int rand, Board const& board, Player const & player, Move &good_move);

	static bool CheckHeroMinionDead(Board & board);

	// include battle cry, and summoning minion
	static bool PlayMinion(Board & board, Card const& card, SlotIndex playing_side, PlayMinionData const& data);

	// no battle cry
	static bool SummonMinion(Card const& card, BoardObjects::MinionIterator & it);

	static bool EquipWeapon(Board & board, Card const& card, SlotIndex playing_side, Move::EquipWeaponData const& data);

public:
	// handle minion/hero attack, calculate damages
	static void HandleAttack(GameEngine::Board & board, GameEngine::SlotIndex attacker_idx, GameEngine::SlotIndex attacked_idx);

	static void DealDamage(GameEngine::Board & board, SlotIndex taker_idx, int damage, bool poisonous);
	static void DealDamage(GameEngine::BoardObjects::BoardObject taker, int damage, bool poisonous);

private:
	static SlotIndex GetTargetForForgetfulAttack(GameEngine::Board & board, SlotIndex origin_attacked);

	static void RemoveMinionsIfDead(Board & board, SlotIndex side);

	static void Fatigue(GameEngine::Board & board, SlotIndex side);
};

} // namespace GameEngine
#endif

#ifndef GAME_ENGINE_CARDS_CARD_GVG_043
#define GAME_ENGINE_CARDS_CARD_GVG_043

DEFINE_CARD_CLASS_START(GVG_043)
// Weapon: Glaivezooka

static void Weapon_BattleCry(GameEngine::Player & equipping_player, SlotIndex)
{
	constexpr int attack_boost = 1;

	const int minion_count = equipping_player.minions.GetMinionCount();
	if (minion_count == 0) return;

	const int minion_chosen = equipping_player.board.random_generator.GetRandom(minion_count);

	auto & buff_target = equipping_player.minions.GetMinion(minion_chosen);
	auto enchant = std::make_unique<Enchantment_BuffMinion_C<attack_boost, 0, 0, 0, false>>();
	buff_target.enchantments.Add(std::move(enchant));
}
DEFINE_CARD_CLASS_END()

#endif

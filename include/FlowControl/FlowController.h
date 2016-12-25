#pragma once

#include "FlowControl/Result.h"
#include "FlowControl/ActionTypes.h"
#include "State/State.h"
#include "FlowControl/FlowContext.h"
#include "FlowControl/IRandomGenerator.h"
#include "FlowControl/ActionParameterWrapper.h"
#include "FlowControl/Contexts.h"
#include "FlowControl/Helpers/Resolver.h"

// Implemention details which depends on manipulators
#include "FlowControl/Manipulators/HeroManipulator-impl.h"
#include "FlowControl/Manipulators/Helpers/AuraHelper-impl.h"
#include "FlowControl/Manipulators/Helpers/EnchantmentHelper-impl.h"
#include "FlowControl/Manipulators/Helpers/OrderedCardsManager-impl.h"
#include "FlowControl/Manipulators/Helpers/DamageHelper-impl.h"
#include "FlowControl/Manipulators/Helpers/HealHelper-impl.h"
#include "State/Cards/Manager-impl.h"

#include "FlowControl/Dispatchers/Minions.h"
#include "FlowControl/Dispatchers/Weapons.h"
#include "Cards/Cards.h"

namespace FlowControl
{
	class FlowController
	{
	public:
		FlowController(state::State & state, IActionParameterGetter & action_parameters, IRandomGenerator & random)
			: state_(state), action_parameters_(action_parameters), random_(random),
			flow_context_(random_, action_parameters_)
		{
		}

		Result PlayCard(int hand_idx)
		{
			action_parameters_.Clear();
			PlayCardInternal(hand_idx);
		
			assert(flow_context_.Empty());
			return flow_context_.result_;
		}

		Result EndTurn()
		{
			action_parameters_.Clear();
			EndTurnInternal();
			
			assert(flow_context_.Empty());
			return flow_context_.result_;
		}

		Result Attack(state::CardRef attacker, state::CardRef defender)
		{
			action_parameters_.Clear();
			AttackInternal(attacker, defender);
			
			assert(flow_context_.Empty());
			return flow_context_.result_;
		}

		Result Resolve()
		{
			Helpers::Resolver(state_, flow_context_).Resolve();
			return flow_context_.result_;
		}

	private:
		void PlayCardInternal(int hand_idx)
		{
			if (!PlayCardPhase(hand_idx)) return;
			if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;
		}

		bool PlayCardPhase(int hand_idx)
		{
			state::CardRef card_ref = state_.GetCurrentPlayer().hand_.Get(hand_idx);
			state::Cards::Card const& card = state_.mgr.Get(card_ref);

			if (card.GetRawData().battlecry_target_getter) {
				if (!(*card.GetRawData().battlecry_target_getter)({ state_, flow_context_, card_ref, card })) {
					flow_context_.result_ = kResultInvalid;
					return false;
				}
			}

			state_.GetCurrentPlayer().resource_.Cost(card.GetCost());

			switch (card.GetCardType())
			{
			case state::kCardTypeMinion:
				return PlayMinionCardPhase(hand_idx, card_ref, card);
			case state::kCardTypeWeapon:
				return PlayWeaponCardPhase(hand_idx, card_ref, card);
			default:
				flow_context_.result_ = kResultInvalid;
				return false;
			}
		}

		bool PlayMinionCardPhase(int hand_idx, state::CardRef card_ref, state::Cards::Card const& card)
		{
			state_.event_mgr.TriggerEvent<state::Events::EventTypes::BeforeMinionSummoned>(
				state::Events::EventTypes::BeforeMinionSummoned::Context{ state_, card_ref, card });

			if (state_.GetCurrentPlayer().minions_.Full()) {
				flow_context_.result_ = kResultInvalid;
				return false;
			}

			int total_minions = (int)state_.GetCurrentPlayer().minions_.Size();
			int put_position = flow_context_.action_parameters_.GetMinionPutLocation(0, total_minions);

			Manipulate(state_, flow_context_).Minion(card_ref).Zone().ChangeTo<state::kCardZonePlay>(state_.current_player, put_position);

			if (card.GetRawData().added_to_play_zone) {
				(*card.GetRawData().added_to_play_zone)({ state_, flow_context_, card_ref, card });
			}

			state_.event_mgr.TriggerEvent<state::Events::EventTypes::OnMinionPlay>(card);

			if (card.GetRawData().battlecry) {
				(*card.GetRawData().battlecry)({state_, flow_context_, card_ref, card});
			}

			state_.event_mgr.TriggerEvent<state::Events::EventTypes::AfterMinionPlayed>(card);

			state_.event_mgr.TriggerEvent<state::Events::EventTypes::AfterMinionSummoned>();
			Manipulate(state_, flow_context_).Minion(card_ref).AfterSummoned();

			return true;
		}

		bool PlayWeaponCardPhase(int hand_idx, state::CardRef card_ref, state::Cards::Card const& card)
		{
			Manipulate(state_, flow_context_).CurrentHero().EquipWeapon(card_ref);

			if (card.GetRawData().added_to_play_zone) {
				(*card.GetRawData().added_to_play_zone)({ state_, flow_context_, card_ref, card });
			}

			if (card.GetRawData().battlecry) {
				(*card.GetRawData().battlecry)({ state_, flow_context_, card_ref, card });
			}

			return true;
		}

	private:
		void AttackInternal(state::CardRef attacker, state::CardRef defender)
		{
			if (!AttackPhase(attacker, defender)) return;
			if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;
		}

		bool AttackPhase(state::CardRef attacker, state::CardRef defender)
		{
			if (!attacker.IsValid()) return SetResult(kResultInvalid);
			if (!defender.IsValid()) return SetResult(kResultInvalid);

			// check if attacker is attackable
			if (!IsAttackable(attacker)) return SetResult(kResultInvalid);

			while (true) {
				// TODO: attacker should not be changed
				state::CardRef origin_attacker = attacker;
				state::CardRef origin_defender = defender;
				state_.event_mgr.TriggerEvent<state::Events::EventTypes::BeforeAttack>(state_, attacker, defender);

				if (!attacker.IsValid()) return kResultNotDetermined;
				if (state_.mgr.Get(attacker).GetHP() <= 0) return kResultNotDetermined;
				if (state_.mgr.Get(attacker).GetZone() != state::kCardZonePlay) return kResultNotDetermined;

				if (!defender.IsValid()) return kResultNotDetermined;
				if (state_.mgr.Get(defender).GetHP() <= 0) return kResultNotDetermined;
				if (state_.mgr.Get(defender).GetZone() != state::kCardZonePlay) return kResultNotDetermined;

				if (origin_attacker == attacker && origin_defender == defender) break;
			}

			state_.event_mgr.TriggerEvent<state::Events::EventTypes::OnAttack>(
				state::Events::EventTypes::OnAttack::Context{ state_, flow_context_, attacker, defender });
			// TODO: attacker lose stealth

			Manipulate(state_, flow_context_).Character(defender).Damage(GetAttackValue(attacker));
			Manipulate(state_, flow_context_).Character(attacker).Damage(GetAttackValue(defender));

			Manipulate(state_, flow_context_).Character(attacker).AfterAttack();

			state_.event_mgr.TriggerEvent<state::Events::EventTypes::AfterAttack>(state_, attacker, defender);

			{
				const state::Cards::Card & attacker_card = state_.mgr.Get(attacker);
				if (attacker_card.GetCardType() == state::kCardTypeHero) {
					state::CardRef weapon_ref = attacker_card.GetRawData().weapon_ref;
					if (weapon_ref.IsValid()) {
						Manipulate(state_, flow_context_).Weapon(weapon_ref).Damage(1);
					}
				}
			}

			return true;
		}

		bool IsAttackable(state::CardRef attacker)
		{
			state::Cards::Card const& card = state_.mgr.Get(attacker);

			if (card.GetCardType() == state::kCardTypeMinion) {
				if (card.GetRawData().enchantable_states.charge == false && card.GetRawData().just_played) return false;
			}

			if (card.GetRawData().num_attacks_this_turn >= 1) return false; // TODO: windfury, etc.

			return true;
		}

		int GetAttackValue(state::CardRef ref)
		{
			int attack = 0;
			
			state::Cards::Card const& card = state_.mgr.Get(ref);

			int v1 = card.GetAttack();
			if (v1 > 0) attack += v1;

			state::CardRef weapon_ref = card.GetRawData().weapon_ref;
			if (weapon_ref.IsValid()) {
				assert(card.GetCardType() == state::kCardTypeHero);

				state::Cards::Card const& weapon = state_.mgr.Get(weapon_ref);
				int v2 = weapon.GetAttack();
				if (v2 > 0) attack += v2;
			}

			return attack;
		}

	private:
		void EndTurnInternal()
		{
			if (state_.turn == 89) {
				flow_context_.result_ = kResultDraw;
				return;
			}
			++state_.turn;

			action_parameters_.Clear();
			EndTurnPhase();
			if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;

			state_.ChangePlayer();

			action_parameters_.Clear();
			StartTurnPhase();
			if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;

			action_parameters_.Clear();
			DrawCardPhase();
			if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;
		}
		void EndTurnPhase()
		{
			state_.event_mgr.TriggerEvent<state::Events::EventTypes::OnTurnEnd>(
				state::Events::EventTypes::OnTurnEnd::Context{ state_, flow_context_ });
		}

		void StartTurnPhase()
		{
			state_.GetCurrentPlayer().resource_.IncreaseTotal();
			state_.GetCurrentPlayer().resource_.Refill();
			// TODO: overload

			state_.event_mgr.TriggerEvent<state::Events::EventTypes::OnTurnStart>();

			for (state::CardRef minion : state_.GetCurrentPlayer().minions_.Get()) {
				Manipulate(state_, flow_context_).Minion(minion).TurnStart();
			}
		}

		void DrawCardPhase()
		{
			Manipulate(state_, flow_context_).CurrentHero().DrawCard();
		}

	private:
		bool SetResult(Result result)
		{
			assert(result != kResultNotDetermined);
			flow_context_.result_ = result;
			return false;
		}

	public:
		state::State & state_;
		ActionParameterWrapper action_parameters_;
		IRandomGenerator & random_;
		FlowContext flow_context_;
	};
}
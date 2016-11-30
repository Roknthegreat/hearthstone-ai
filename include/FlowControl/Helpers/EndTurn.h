#pragma once

#include "FlowControl/Result.h"
#include "FlowControl/Manipulate.h"
#include "FlowControl/Helpers/DamageDealer.h"
#include "FlowControl/Helpers/Resolver.h"
#include "FlowControl/Manipulators/HeroManipulator.h"
#include "FlowControl/IRandomGenerator.h"
#include "FlowControl/ActionParameterWrapper.h"

namespace FlowControl
{
	namespace Helpers
	{
		class OnTurnEnd
		{
		public:
			OnTurnEnd(state::State & state, FlowContext & flow_context)
				: state_(state), flow_context_(flow_context)
			{

			}

			Result Go()
			{
				Result rc = kResultNotDetermined;

				state_.event_mgr.TriggerEvent<state::Events::EventTypes::OnTurnEnd>();
				if ((rc = Resolver(state_, flow_context_).Resolve()) != kResultNotDetermined) return rc;

				if (state_.turn == 89) return kResultDraw;
				++state_.turn;

				state_.ChangePlayer();

				state_.GetCurrentPlayer().resource_.IncreaseTotal();
				state_.GetCurrentPlayer().resource_.Refill();
				// TODO: overload

				state_.event_mgr.TriggerEvent<state::Events::EventTypes::OnTurnStart>();
				if ((rc = Resolver(state_, flow_context_).Resolve()) != kResultNotDetermined) return rc;

				Manipulate(state_, flow_context_).CurrentHero().DrawCard();
				if ((rc = Resolver(state_, flow_context_).Resolve()) != kResultNotDetermined) return rc;

				return kResultNotDetermined;
			}

		private:
			Helpers::DamageDealer GetDamageDealer() { return DamageDealer(state_, flow_context_); }

		private:
			state::State & state_;
			FlowContext & flow_context_;
		};
	}
}
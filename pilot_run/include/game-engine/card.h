#ifndef GAME_ENGINE_CARD_H
#define GAME_ENGINE_CARD_H

#include <stdexcept>
#include <functional>
#include <string>
#include <sstream>
#include "common.h"

namespace GameEngine {

class Card
{
	public:
		enum Rarity
		{
			RARITY_FREE,
			RARITY_COMMON,
			RARITY_RARE,
			RARITY_EPIC,
			RARITY_LEGENDARY,
			RARITY_UNKNOWN
		};

		enum SpecialCardIDs
		{
			SPECIAL_CARD_INVALID = -1,
			SPECIAL_CARD_NORMAL_DRAW = -2,
			SPECIAL_CARD_MULLIGAN_KEPT = -3
		};

	public:
		bool operator==(const Card &rhs) const;
		bool operator!=(const Card &rhs) const;

		void MarkInvalid();
		bool IsValid() const;
		std::string GetDebugString() const;

	public:
		int id;

		Rarity rarity;

		enum Type {
			TYPE_INVALID = 0, // the smallest must be zero
			TYPE_MINION,
			TYPE_SPELL,
			TYPE_WEAPON,
			TYPE_HEROPOWER,
			TYPE_SECRET,
			TYPE_MAX = TYPE_SECRET // must be the largest
		} type;

		enum MinionRace {
			RACE_NORMAL,
			RACE_BEAST,
			RACE_DEMON,
			RACE_DRAGON,
			RACE_MECH,
			RACE_MURLOC,
			RACE_PIRATE,
			RACE_TOTEM
		};

		int cost;

		struct Minion {
			int attack;
			int hp;
			MinionRace race;
			bool taunt;
			bool charge;
			bool shield;
			bool stealth;
			bool forgetful;
			bool freeze;
			bool windfury;
			bool poisonous;

			int spell_damage;

			void Clear() {
				this->attack = 0;
				this->hp = 0;
				this->race = RACE_NORMAL;
				this->taunt = false;
				this->charge = false;
				this->shield = false;
				this->stealth = false;
				this->forgetful = false;
				this->freeze = false;
				this->windfury = false;
				this->poisonous = false;
				this->spell_damage = 0;
			}

			bool operator==(const Minion &rhs) const {
				if (this->attack != rhs.attack) return false;
				if (this->hp != rhs.hp) return false;
				if (this->race != rhs.race) return false;
				if (this->taunt != rhs.taunt) return false;
				if (this->charge != rhs.charge) return false;
				if (this->shield != rhs.shield) return false;
				if (this->stealth != rhs.stealth) return false;
				if (this->forgetful != rhs.forgetful) return false;
				if (this->freeze != rhs.freeze) return false;
				if (this->windfury != rhs.windfury) return false;
				if (this->poisonous != rhs.poisonous) return false;
				if (this->spell_damage != rhs.spell_damage) return false;
				return true;
			}

			bool operator!=(const Minion &rhs) const {
				return !(*this == rhs);
			}
		};

		struct Spell {
			void Clear() {
			}
		};

		struct Weapon {
			int attack;
			int durability;
			bool forgetful;
			bool freeze;
			bool windfury;

			bool operator==(const Weapon &rhs) const {
				if (this->attack != rhs.attack) return false;
				if (this->durability != rhs.durability) return false;
				return true;
			}

			bool operator!=(const Weapon &rhs) const {
				return !(*this == rhs);
			}
		};

		struct Secret {
			// TODO
		};

		union {
			Minion minion;
			Spell spell;
			Weapon weapon;
			Secret secret;
		} data;
};

inline bool Card::IsValid() const
{
	return this->id > 0;
}

inline void Card::MarkInvalid()
{
	this->id = SPECIAL_CARD_INVALID;
}

inline std::string Card::GetDebugString() const
{
	if (!this->IsValid()) {
		switch (this->id) {
		case SPECIAL_CARD_INVALID:
			return "INVALID";
		case SPECIAL_CARD_NORMAL_DRAW:
			return "NORMAL_DRAW";
		case SPECIAL_CARD_MULLIGAN_KEPT:
			return "MULLIGAN_KEPT";
		}
		return "UNKNOWN";
	}

	std::ostringstream oss;

	if (this->type == TYPE_MINION)
	{
		oss << "M" << this->cost << this->data.minion.attack << this->data.minion.hp;
	}
	else if (this->type == TYPE_SPELL)
	{
		oss << "S" << this->cost;
	}
	else if (this->type == TYPE_WEAPON)
	{
		oss << "W" << this->cost << this->data.weapon.attack << this->data.weapon.durability;
	}
	else {
		oss << "???";
	}

	return oss.str();
}

inline bool Card::operator==(const Card &rhs) const
{
	if (this->id != rhs.id) return false;

#ifdef DEBUG
	std::function<bool(Card const&)> is_same = [this](Card const& rhs) {
		if (this->type != rhs.type) return false;
		if (this->cost != rhs.cost) return false;

		switch (this->type) {
		case TYPE_MINION:
			if (this->data.minion != rhs.data.minion) return false;
			break;

		case TYPE_WEAPON:
			if (this->data.weapon != rhs.data.weapon) return false;
			break;

		case TYPE_INVALID:
		case TYPE_SPELL:
		case TYPE_SECRET:
			break;
		}
		return true;
	};
	if (is_same(rhs) == false) {
		throw std::runtime_error("card id is identical, but with different stats");
	}
#endif

	return true;
}

inline bool Card::operator!=(const Card &rhs) const
{
	return !(*this == rhs);
}

} // namespace GameEngine

namespace std {
	template <> struct hash<GameEngine::Card::Minion> {
		typedef GameEngine::Card::Minion argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type &s) const {
			result_type result = 0;

			GameEngine::hash_combine(result, s.attack);
			GameEngine::hash_combine(result, s.hp);
			GameEngine::hash_combine(result, (int)s.race);
			GameEngine::hash_combine(result, s.taunt);
			GameEngine::hash_combine(result, s.charge);
			GameEngine::hash_combine(result, s.shield);
			GameEngine::hash_combine(result, s.stealth);
			GameEngine::hash_combine(result, s.forgetful);
			GameEngine::hash_combine(result, s.freeze);
			GameEngine::hash_combine(result, s.windfury);
			GameEngine::hash_combine(result, s.poisonous);

			return result;
		}
	};

	template <> struct hash<GameEngine::Card::Weapon> {
		typedef GameEngine::Card::Weapon argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type &s) const {
			result_type result = 0;

			GameEngine::hash_combine(result, s.attack);
			GameEngine::hash_combine(result, s.durability);

			return result;
		}
	};

	template <> struct hash<GameEngine::Card> {
		typedef GameEngine::Card argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type &s) const {
			result_type result = 0;

			GameEngine::hash_combine(result, s.id);

			// skip hash for other fields
			// since there should no duplicate card ids with different stats

			return result;
		}
	};

}

#endif
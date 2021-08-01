#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/entity.h"
#include "include/hud.h"
#include "include/move.h"
#include "libs/vwf.h"

const char attack_message[] = "%s used %s!";
const char missed_message[] = "%s missed.";
const char defeat_message[] = "%s was defeated.\n%s gained %u experience.\n%u/%u";

void use_melee_move(entity *self, move *self_move)
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self_move->bank);

	// Update positions before using them to attack.
	move_entities();
	vec8 target_pos = {self->x_pos, self->y_pos};
	int8_t result;
	entity *target = NULL;
	for (uint8_t i = 0; i < self_move->data->range; i++) {
		move_direction(&target_pos, self->direction);
		result = check_entity_at(target_pos.x, target_pos.y);
		if (result < 3 && self->team != ENEMY_TEAM)
			continue;
		if (result != -1) {
			target = &entities[result];
			break;
		}
	}

	// Construct attack message
	char message[MESSAGE_SIZE];
	// Entity names are stored in RAM during gameplay, so this is safe.
	sprintf(message, attack_message, self->name, self_move->data->name);
	vwf_wrap_str(20 * 8, message);
	print_hud(message);

	attack_animation(self);

	if (!target) {
		sprintf(message, missed_message, self->name);
		vwf_wrap_str(20 * 8, message);
		print_hud(message);
		return;
	}

	uint16_t damage = self_move->data->power + get_attack_bonus(self) - get_defense_bonus(target);
	if (target->health <= damage) {
		defeat_animation(target);
		if (self->team != ENEMY_TEAM) {
			SWITCH_ROM_MBC1(target->bank);
			uint16_t reward = get_xp_reward(target);
			self->xp += reward;
			sprintf(message, defeat_message, target->name,
				self->name, reward, self->xp, 
				get_xp_threshold(self->level));
			vwf_wrap_str(20 * 8, message);
			print_hud(message);
			memset(target, 0, sizeof(entity));
			if (self->xp >= get_xp_threshold(self->level)) {
				self->xp -= get_xp_threshold(self->level);
				self->level++;
				//sprintf(message, "%s leveled up to level %u",
				//	self->name, self->level);
				//vwf_wrap_str(20 * 8, message);
				//print_hud(message);
			}
		}
	} else {
		hurt_animation(target);
		target->health -= damage;
	}

	SWITCH_ROM_MBC1(temp_bank);
}
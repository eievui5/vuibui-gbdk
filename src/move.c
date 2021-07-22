// Cannot bank this file, It relies too much on formating strings from various
// sources.

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/entity.h"
#include "include/hud.h"
#include "include/move.h"
#include "libs/vwf.h"

const char attack_message[] = "%s used %s!";
const char missed_message[] = "%s missed.";
const char defeat_message[] = "%s was defeated.";

void use_melee_move(entity *self, move *self_move)
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self_move->bank);

	// Update positions before using them to attack.
	move_entities();
	vec8 target_pos = {self->x_pos, self->y_pos};
	entity *target = NULL;
	for (uint8_t i = 0; i < self_move->data->range; i++) {
		move_direction(&target_pos, self->direction);
		target = check_entity_at(target_pos.x, target_pos.y);
		if (target)
			break;
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

	if (target->health <= self_move->data->power) {
		defeat_animation(target);
		sprintf(message, defeat_message, target->name);
		vwf_wrap_str(20 * 8, message);
		print_hud(message);
		memset(target, 0, sizeof(entity));
	} else {
		hurt_animation(target);
		target->health -= self_move->data->power;
	}

	SWITCH_ROM_MBC1(temp_bank);
}
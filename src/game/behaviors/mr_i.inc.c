// mr_i.inc.c

// this is actually the MrI particle loop function. piranha
// plant code later on reuses this function.
void bhv_piranha_particle_loop(void) {
    if (o->oTimer == 0) {
        o->oVelY = 20.0f + 20.0f * random_float();
        o->oForwardVel = 20.0f + 20.0f * random_float();
        o->oMoveAngleYaw = random_u16();
    }
    cur_obj_move_using_fvel_and_gravity();
}

void mr_i_piranha_particle_act_0(void) {
    cur_obj_scale(3.0f);
    o->oForwardVel = 20.0f;
    cur_obj_update_floor_and_walls();

    if (o->oInteractStatus & INT_STATUS_INTERACTED) {
        o->oAction = 1;
    } else if ((o->oTimer > 100) || (o->oMoveFlags & OBJ_MOVE_HIT_WALL)
               || (o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        obj_mark_for_deletion(o);
        spawn_mist_particles();
    }
}

void mr_i_piranha_particle_act_1(void) {
    s32 i;
    obj_mark_for_deletion(o);
    for (i = 0; i < 10; i++) {
        spawn_object(o, MODEL_PURPLE_MARBLE, bhvPurpleParticle);
    }
}

void (*sMrIParticleActions[])(void) = {
    mr_i_piranha_particle_act_0,
    mr_i_piranha_particle_act_1,
};

void bhv_mr_i_particle_loop(void) {
    cur_obj_call_action_function(sMrIParticleActions);
}

void spawn_mr_i_particle(void) {
    struct Object *particle;
    f32 mr_i_size = o->header.gfx.scale[1];

    // sets particle spawn point to 50u above Mr I and 90u forwards
    particle = spawn_object(o, MODEL_PURPLE_MARBLE, bhvMrIParticle);
    particle->oPosY += 50.0f * mr_i_size;
    particle->oPosX += sins(o->oMoveAngleYaw) * 90.0f * mr_i_size;
    particle->oPosZ += coss(o->oMoveAngleYaw) * 90.0f * mr_i_size;

    cur_obj_play_sound_2(SOUND_OBJ_MRI_SHOOT);
}

void bhv_mr_i_body_loop(void) {
    obj_copy_pos_and_angle(o, o->parentObj);

    if (!(o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        obj_copy_scale(o, o->parentObj);
        obj_set_parent_relative_pos(o, 0, 0, o->header.gfx.scale[1] * 100.0f);
        obj_build_transform_from_pos_and_angle(o, 44, 15);
        obj_translate_local(o, 6, 44);
        o->oFaceAnglePitch = o->oMoveAnglePitch;
        o->oGraphYOffset = o->header.gfx.scale[1] * 100.0f;
    }

    if (o->parentObj->oMrIUnk110 != 1) {
        o->oAnimState = -1;
    } else {
        o->oAnimState++;
        if (o->oAnimState == 15) {
            o->parentObj->oMrITurnDirection = 0;
        }
    }

    if (o->parentObj->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
        obj_mark_for_deletion(o);
    }
}

// death action
void mr_i_act_3(void) {
    s16 move_angle_yaw;
    s16 turn_direction_factor;
    f32 scale_factor;
    f32 turning_speed;
    UNUSED u8 filler[8];
    f32 scale;
    f32 big_mr_i_factor;

    if (o->oBhvParams2ndByte != 0) {
        big_mr_i_factor = 2.0f;
    } else {
        big_mr_i_factor = 1.0f;
    }

    if (o->oMrITurnDirection < 0) {
        turn_direction_factor = 0x1000;
    } else {
        turn_direction_factor = -0x1000;
    }

    turning_speed = (o->oTimer + 1) / 96.0f;

    if (o->oTimer < 64) {
        move_angle_yaw = o->oMoveAngleYaw;
        o->oMoveAngleYaw += turn_direction_factor * coss(0x4000 * turning_speed);

        if (move_angle_yaw < 0 && o->oMoveAngleYaw >= 0) {
            cur_obj_play_sound_2(SOUND_OBJ2_MRI_SPINNING);
        }

        o->oMoveAnglePitch = (1.0 - coss(0x4000 * turning_speed)) * -0x4000;
        cur_obj_shake_y(4.0f);
    } else if (o->oTimer < 96) {
        if (o->oTimer == 64) {
            cur_obj_play_sound_2(SOUND_OBJ_MRI_DEATH);
        }

        scale_factor = (f32)(o->oTimer - 63) / 32;
        o->oMoveAngleYaw += turn_direction_factor * coss(0x4000 * turning_speed);
        o->oMoveAnglePitch = (1.0 - coss(0x4000 * turning_speed)) * -0x4000;

        cur_obj_shake_y((s32)((1.0f - scale_factor) * 4)); // trucating the f32?
        scale = coss(0x4000 * scale_factor) * 0.4 + 0.6;
        cur_obj_scale(scale * big_mr_i_factor);
    } else if (o->oTimer < 104) {
        // do nothing
    } else if (o->oTimer < 168) {
        if (o->oTimer == 104) {
            cur_obj_become_intangible();
            spawn_mist_particles();
            o->oMrIScale = big_mr_i_factor * 0.6;
            if (o->oBhvParams2ndByte != 0) {
                o->oPosY += 100.0f;
                spawn_default_star(1370, 2000.0f, -320.0f);
                obj_mark_for_deletion(o);
            } else {
                cur_obj_spawn_loot_blue_coin();
            }
        }

        o->oMrIScale -= 0.2 * big_mr_i_factor;

        if (o->oMrIScale < 0) {
            o->oMrIScale = 0;
        }

        cur_obj_scale(o->oMrIScale);
    } else {
        obj_mark_for_deletion(o);
    }
}

// locked onto Mario action
void mr_i_act_2(void) {
    s16 move_angle_yaw = o->oMoveAngleYaw;
    s16 move_angle_delta;

    if (o->oTimer == 0) {
        if (o->oBhvParams2ndByte != 0) {
            o->oMrITurnCountdown = 200;
        } else {
            o->oMrITurnCountdown = 120;
        }
        o->oMrITurnDistance = 0;
        o->oMrITurnDirection = 0;
        o->oMrIShotTimer = 0;
    }

    obj_turn_toward_object(o, gMarioObject, O_MOVE_ANGLE_YAW_INDEX, 0x800);
    obj_turn_toward_object(o, gMarioObject, O_MOVE_ANGLE_PITCH_INDEX, 0x400);

    move_angle_delta = move_angle_yaw - (s16)(o->oMoveAngleYaw);

    if (move_angle_delta == 0) {
        o->oMrITurnDistance = 0;
        o->oMrITurnDirection = 0;
    } else if (move_angle_delta > 0) {
        if (o->oMrITurnDirection > 0) {
            o->oMrITurnDistance += move_angle_delta;
        } else {
            o->oMrITurnDistance = 0;
        }
        o->oMrITurnDirection = 1;
    } else {
        if (o->oMrITurnDirection < 0) {
            o->oMrITurnDistance -= move_angle_delta;
        } else {
            o->oMrITurnDistance = 0;
        }
        o->oMrITurnDirection = -1;
    }

    if (o->oMrITurnDistance == 0) {
        o->oMrITurnCountdown = 120;
    }
    if (o->oMrITurnDistance > 65536) {
        o->oAction = 3;
    }

    o->oMrITurnCountdown--;

    if (o->oMrITurnCountdown == 0) {
        o->oMrITurnCountdown = 120;
        o->oMrITurnDistance = 0;
    }

    if (o->oMrITurnDistance < 5000) {
        if (o->oMrIShotTimer == o->oMrIShotTimerMax) {
            o->oMrIBlinking = 1;
        }

        if (o->oMrIShotTimer == o->oMrIShotTimerMax + 20) {
            spawn_mr_i_particle();
            o->oMrIShotTimer = 0;
            o->oMrIShotTimerMax = (s32)(random_float() * 50.0f + 50.0f);
        }
        o->oMrIShotTimer++;
    } else {
        o->oMrIShotTimer = 0;
        o->oMrIShotTimerMax = (s32)(random_float() * 50.0f + 50.0f);
    }

    if (o->oDistanceToMario > 800.0f) {
        o->oAction = 1;
    }
}

// casually turning around action
void mr_i_act_1(void) {
    s16 angle_to_mario = obj_angle_to_object(o, gMarioObject);
    s16 angle_to_mario_diff = abs_angle_diff(o->oMoveAngleYaw, angle_to_mario);
    s16 eye_contact_diff = abs_angle_diff(o->oMoveAngleYaw, gMarioObject->oFaceAngleYaw);

    if (o->oTimer == 0) {
        cur_obj_become_tangible();
        o->oMoveAnglePitch = 0;
        o->oMrIShotTimer = 30;
        o->oMrIShotTimerMax = random_float() * 20.0f;
        if (o->oMrIShotTimerMax & 1) {
            o->oAngleVelYaw = -256;
        } else {
            o->oAngleVelYaw = 256;
        }
    }

    if (angle_to_mario_diff < 1024 && eye_contact_diff > 0x4000) {
        if (o->oDistanceToMario < 700.0f) {
            o->oAction = 2;
        } else {
            o->oMrIShotTimer++;
        }
    } else {
        o->oMoveAngleYaw += o->oAngleVelYaw;
        o->oMrIShotTimer = 30;
    }

    if (o->oMrIShotTimer == o->oMrIShotTimerMax + 60) {
        o->oMrIUnk110 = 1;
    }

    if (o->oMrIShotTimer > o->oMrIShotTimerMax + 80) {
        o->oMrIShotTimer = 0;
        o->oMrIShotTimerMax = random_float() * 80.0f;
        spawn_mr_i_particle();
    }
}

// Mr I init action
void mr_i_act_0(void) {
#ifndef VERSION_JP
    obj_set_angle(o, 0, 0, 0);
#else
    o->oMoveAnglePitch = 0;
    o->oMoveAngleYaw = 0;
    o->oMoveAngleRoll = 0;
#endif
    cur_obj_scale(o->oBhvParams2ndByte + 1);

    if (o->oTimer == 0) {
        cur_obj_set_pos_to_home();
    }

    if (o->oDistanceToMario < 1500.0f) {
        o->oAction = 1;
    }
}

void (*sMrIActions[])(void) = {
    mr_i_act_0,
    mr_i_act_1,
    mr_i_act_2,
    mr_i_act_3,
};

struct ObjectHitbox sMrIHitbox = {
    /* interactType:      */ INTERACT_DAMAGE,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 2,
    /* health:            */ 2,
    /* numLootCoins:      */ 5,
    /* radius:            */ 80,
    /* height:            */ 150,
    /* hurtboxRadius:     */ 0,
    /* hurtboxHeight:     */ 0,
};

void bhv_mr_i_loop(void) {
    obj_set_hitbox(o, &sMrIHitbox);
    cur_obj_call_action_function(sMrIActions);

    if (o->oAction != 3) {
        if ((o->oDistanceToMario > 3000.0f) || (o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
            o->oAction = 0;
        }
    }

    o->oInteractStatus = 0;
}

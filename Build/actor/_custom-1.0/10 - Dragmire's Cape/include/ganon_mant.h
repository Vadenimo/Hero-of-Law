#ifndef Z_EN_GANON_MANT_H
#define Z_EN_GANON_MANT_H

#include <z64hdr/oot_u10/z64hdr.h>

struct EnGanonMant;

#define GANON_MANT_NUM_JOINTS 12
#define GANON_MANT_NUM_STRANDS 12

#define ACTOR_FLAG_ATTENTION_ENABLED (1 << 0)
#define ACTOR_FLAG_HOSTILE (1 << 2)
#define ACTOR_FLAG_FRIENDLY (1 << 3)
#define ACTOR_FLAG_UPDATE_CULLING_DISABLED (1 << 4)
#define ACTOR_FLAG_DRAW_CULLING_DISABLED (1 << 5)
#define ACTOR_FLAG_INSIDE_CULLING_VOLUME (1 << 6)
#define ACTOR_FLAG_REACT_TO_LENS (1 << 7)
#define ACTOR_FLAG_TALK (1 << 8)
#define ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR (1 << 9)
#define ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER (1 << 10)
#define ACTOR_FLAG_GRASS_DESTROYED (1 << 11)
#define ACTOR_FLAG_IGNORE_QUAKE (1 << 12)
#define ACTOR_FLAG_HOOKSHOT_ATTACHED (1 << 13)
#define ACTOR_FLAG_CAN_ATTACH_TO_ARROW (1 << 14)
#define ACTOR_FLAG_ATTACHED_TO_ARROW (1 << 15)
#define ACTOR_FLAG_TALK_OFFER_AUTO_ACCEPTED (1 << 16)
#define ACTOR_FLAG_CARRY_X_ROT_INFLUENCE (1 << 17)
#define ACTOR_FLAG_TALK_WITH_C_UP (1 << 18)
#define ACTOR_FLAG_SFX_ACTOR_POS_2 (1 << 19) // see Actor_PlaySfx_Flagged2
#define ACTOR_AUDIO_FLAG_SFX_CENTERED_1 (1 << 20) // see Actor_PlaySfx_FlaggedCentered1
#define ACTOR_AUDIO_FLAG_SFX_CENTERED_2 (1 << 21) // see Actor_PlaySfx_FlaggedCentered2
#define ACTOR_FLAG_IGNORE_POINT_LIGHTS (1 << 22)
#define ACTOR_FLAG_THROW_ONLY (1 << 23)
#define ACTOR_FLAG_SFX_FOR_PLAYER_BODY_HIT (1 << 24)
#define ACTOR_FLAG_UPDATE_DURING_OCARINA (1 << 25)
#define ACTOR_FLAG_CAN_PRESS_SWITCHES (1 << 26)
#define ACTOR_FLAG_LOCK_ON_DISABLED (1 << 27)
#define ACTOR_FLAG_SFX_TIMER (1 << 28)


#define MATRIX_FINALIZE_AND_LOAD(pkt, gfxCtx, file, line) \
    gSPMatrix(pkt, Matrix_NewMtx(gfxCtx, file, line), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW)


typedef struct MantStrand {
    /* 0x000 */ Vec3f root;                              // root position along the collar
    /* 0x00C */ Vec3f joints[GANON_MANT_NUM_JOINTS];     // "joints" for deforming the cloak, stemming from root and propagating down the cloak
    /* 0x090 */ Vec3f rotations[GANON_MANT_NUM_JOINTS];  // normal vector rotations, x and y only
    /* 0x120 */ Vec3f velocities[GANON_MANT_NUM_JOINTS]; // velocities calculated as differences in position from last update
    /* 0x1B0 */ u8 torn[GANON_MANT_NUM_JOINTS];          // guess: whether the joint is torn?
} MantStrand; // size = 0x1C8

typedef struct EnGanonMant {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ MantStrand strands[GANON_MANT_NUM_STRANDS];
    /* 0x16AC */ f32 minY; // minimum Y value possible for joints, for emulating collision with a floor
    /* 0x16B0 */ f32 backPush; // larger values push the cloak further outwards, negative is away from the actor
    /* 0x16B4 */ f32 backSwayMagnitude; // magnitude of backwards/forwards swaying
    /* 0x16B8 */ f32 sideSwayMagnitude; // magnitude of sideways swaying
    /* 0x16BC */ f32 attachRightArmTimer; // timer for the duration of which the cloak is attached to right forearm and left shoulder
    /* 0x16C0 */ f32 attachLeftArmTimer; // timer for the duration of which the cloak is attached to left forearm and right shoulder
    /* 0x16C4 */ f32 attachShouldersTimer; // timer for the duration of which the cloak is attached to both shoulders
    /* 0x16C8 */ f32 gravity; // strand gravity
    /* 0x16CC */ f32 baseYaw;
    /* 0x16D0 */ f32 minDist; // closest distance the cloak can get to the attached actor without being pushed away
    /* 0x16D4 */ Vec3f rightForearmPos;
    /* 0x16E0 */ Vec3f leftForearmPos;
    /* 0x16EC */ Vec3f rightShoulderPos;
    /* 0x16F8 */ Vec3f leftShoulderPos;
    /* 0x1704 */ u8 tearTimer; // tear the cloak for x many frames
    /* 0x1705 */ u8 updateHasRun;
    /* 0x1706 */ u8 frameTimer;
} EnGanonMant; // size = 0x1708

#endif

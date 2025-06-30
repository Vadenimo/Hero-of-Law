#include "ultra64.h"
#include "global.h"

Vtx Hair_Lower_Hair_Lower_mesh_layer_Opaque_vtx_cull[8] = {
	{{ {-412, -3021, -280}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-412, -3021, 503}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-412, 1662, 503}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-412, 1662, -280}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {412, -3021, -280}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {412, -3021, 503}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {412, 1662, 503}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {412, 1662, -280}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Vtx Hair_Lower_Hair_Lower_mesh_layer_Opaque_vtx_0[12] = {
	{{ {0, 1662, 503}, 0, {434, 738}, {226, 209, 114, 255} }},
	{{ {-412, 680, -280}, 0, {488, 887}, {140, 235, 48, 255} }},
	{{ {0, -3021, 59}, 0, {434, 1573}, {0, 244, 126, 255} }},
	{{ {412, 680, -280}, 0, {380, 887}, {95, 232, 80, 255} }},
	{{ {0, 1662, 503}, 0, {434, 738}, {14, 244, 130, 255} }},
	{{ {0, -3021, 59}, 0, {434, 1573}, {0, 244, 126, 255} }},
	{{ {412, 680, -280}, 0, {380, 887}, {87, 238, 165, 255} }},
	{{ {0, -3021, 59}, 0, {434, 1573}, {0, 244, 130, 255} }},
	{{ {-412, 680, -280}, 0, {488, 887}, {0, 247, 129, 255} }},
	{{ {-412, 680, -280}, 0, {488, 887}, {253, 80, 157, 255} }},
	{{ {0, 1662, 503}, 0, {434, 738}, {156, 177, 4, 255} }},
	{{ {412, 680, -280}, 0, {380, 887}, {6, 245, 130, 255} }},
};

Gfx Hair_Lower_Hair_Lower_mesh_layer_Opaque_tri_0[] = {
	gsSPVertex(Hair_Lower_Hair_Lower_mesh_layer_Opaque_vtx_0 + 0, 12, 0),
	gsSP2Triangles(0, 1, 2, 0, 3, 4, 5, 0),
	gsSP2Triangles(6, 7, 8, 0, 9, 10, 11, 0),
	gsSPEndDisplayList(),
};

Gfx mat_Hair_Lower_GoddessFast64_layerOpaque[] = {
	gsSPLoadGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
	gsDPPipeSync(),
	gsDPSetCombineLERP(TEXEL0, PRIMITIVE, PRIM_LOD_FRAC, TEXEL0, 0, 0, 0, 1, PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT, 0, 0, 0, 1),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOISE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_NONE | G_ZS_PIXEL | G_RM_PASS | G_RM_AA_ZB_TEX_EDGE2),
	gsSPTexture(3000, 3000, 0, 0, 1),
	gsDPSetPrimColor(128, 128, 255, 255, 170, 255),
	gsDPSetEnvColor(120, 110, 0, 255),
	gsDPSetTextureImage(G_IM_FMT_I, G_IM_SIZ_8b_LOAD_BLOCK, 1, texturei8),
	gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_8b_LOAD_BLOCK, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadBlock(7, 0, 0, 511, 512),
	gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_8b, 4, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 1, G_TX_WRAP | G_TX_NOMIRROR, 5, 1),
	gsDPSetTileSize(0, 0, 0, 124, 124),
	gsSPEndDisplayList(),
};

Gfx Hair_Lower[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(Hair_Lower_Hair_Lower_mesh_layer_Opaque_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_Hair_Lower_GoddessFast64_layerOpaque),
	gsSPDisplayList(Hair_Lower_Hair_Lower_mesh_layer_Opaque_tri_0),
	gsSPEndDisplayList(),
};


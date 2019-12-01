#include "ts.h"
#include "key.h"

#include <citro2d.h>
#include <string.h>

// #define TS_DEBUG

#ifdef TS_DEBUG
#include <stdio.h>
#define debug_print(fmt, args ...) printf(fmt, ##args)
#define r(fmt, args ...) printf(fmt, ##args)
#else
#define debug_print(fmt, args ...)
#define r(fmt, args ...)
#endif

AM_TitleMediaEntry titleEntry;

static u32 titleCount;
static s32 titleCurrent;
static AM_TitleMediaEntry* titleList;
static C2D_Image** titleIcons;

/**
 * @brief Updates the output title enty.
 */
static void TS_Select(void)
{
	debug_print("TS_Select:\n");

	titleEntry = titleList[titleCurrent];

	debug_print("Selected: %s\n[0x%014llx]\n", AM_GetPokemonTitleName(titleEntry.titleid), titleEntry.titleid);
}

/**
 * @brief Initializes the title module.
 */
static Result TS_Init(void)
{
	Result ret;

	debug_print("TS_Init:\n");

	titleCount = 0;
	titleCurrent = 0;

	amInit();

	ret = AM_GetPokemonTitleEntryList(&titleList, &titleCount);
	r(" > AM_GetPokemonTitleEntryList: %lx\n", ret);

	debug_print("Got: %li titles\n", titleCount);

	titleIcons = (C2D_Image**) malloc(titleCount * sizeof(C2D_Image*));

	for (u32 i = 0; i < titleCount; i++)
	{
		// TODO: Add security checkers.
		debug_print("Texturing %li\n", i);

		titleIcons[i] = calloc(1, sizeof(*titleIcons[i]));
		C3D_TexInit(titleIcons[i]->tex, 64, 64, 3);	// 3 is TEXFMT_RGB565 in sf2dlib
		C3D_TexSetWrap(titleIcons[i]->tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
		*titleIcons[i]->subtex = { 48, 48, 0.0f, 1.0f, 1.0f, 0.0f };

		u16* dst = (u16*)(titleIcons[i]->tex->data + 64 * 8 * 2 * sizeof(u16));
		u16* src = (u16*)(titleList[i].smdh->bigIconData);
		for (u8 j = 0; j < 48; j += 8)
		{
			memcpy(dst, src, 48 * 8 * sizeof(u16));
			src += 48 * 8;
			dst += 64 * 8;
		}
	}

	debug_print("Textured\n");

	if (titleCount > 0)
	{
		TS_Select();
	}

	return ret;
}

/**
 * @brief Exits the title module.
 */
static void TS_Exit(void)
{
	AM_FreePokemonTitleEntryList(titleList, titleCount);
	free(titleIcons);

	amExit();
}
/**
 * @brief Selects the previous title.
 */
static void TS_Prev(void)
{
	titleCurrent--;
	if (titleCurrent < 0)
		titleCurrent = titleCount-1;
	TS_Select();
}

/**
 * @brief Selects the next title.
 */
static void TS_Next(void)
{
	titleCurrent++;
	if (titleCurrent > titleCount-1)
		titleCurrent = 0;
	TS_Select();
}

bool TS_Loop(void)
{
	TS_Init();

	// If no title, cancel it.
	if (titleCount == 0)
	{
		TS_Exit();
		return false;
	}
	// If only one title, select it by default.
	// else if (titleCount == 1)
	// {
	// 	TS_Select();
	// 	TS_Exit();
	// 	return true;
	// }

	bool tsReturn = false;

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kState = hidKeysDown();

		if (kState & (KEY_UP | KEY_LEFT))
		{
			TS_Prev();
		}
		if (kState & (KEY_DOWN | KEY_RIGHT))
		{
			TS_Next();
		}

		if (kState & KEY_A)
		{
			tsReturn = true;
			break;
		}

		if (kState & (KEY_B | KEY_START))
		{
			tsReturn = false;
			break;
		}

#ifdef TS_DEBUG
		gspWaitForVBlank();
#else
		C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
		C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
		u32 white = C2D_Color32(0xf8, 0xf8, 0xf8, 0xff);

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, white);

		C2D_SceneBegin(top);
		{
			// Center
			if (titleCount > 0)
				C2D_DrawImageAt(*titleIcons[titleCurrent], 152.0f, 160.0f, 0.0f, NULL, 2.0f, -2.0f);

			// Left
			if (titleCurrent > 0)
				C2D_DrawImageAt(*titleIcons[titleCurrent-1], 64.0f, 144.0f, 0.0f, NULL, 1.0f, -1.0f);
			else if (titleCurrent < titleCount-1)
				C2D_DrawImageAt(*titleIcons[titleCount-1], 64.0f, 144.0f, 0.0f, NULL, 1.0f, -1.0f);

			// Right
			if (titleCurrent < titleCount-1)
				C2D_DrawImageAt(*titleIcons[titleCurrent+1], 288.0f, 144.0f, 0.0f, NULL, 1.0f, -1.0f);
			else if (titleCurrent > 0)
				C2D_DrawImageAt(*titleIcons[0], 288.0f, 144.0f, 0.0f, NULL, 1.0f, -1.0f);
		}
		C3D_FrameEnd(0);
#endif
	}

	TS_Exit();
	return tsReturn;
}

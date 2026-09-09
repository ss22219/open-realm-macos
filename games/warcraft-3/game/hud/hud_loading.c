/* Serializes the stock Loading.fdf tree before gameplay UI exists. */
#include "hud_local.h"

void UI_LoadHudLoading(void) {
    if (!LoadingScreen_Load(&hud.loading)) {
        fprintf(stderr, "UI_LoadHudLoading: missing Loading.fdf\n");
        return;
    }
    if (hud.loading.LoadingCustomPanel) UI_SetHidden(hud.loading.LoadingCustomPanel, false);
    if (hud.loading.LoadingMeleePanel) UI_SetHidden(hud.loading.LoadingMeleePanel, true);
    if (hud.loading.LoadingBar) {
        UI_SetPortraitFrameModel(hud.loading.LoadingBar, UI_LoadModel("LoadingProgressBar", true));
        hud.loading.LoadingBar->Type = FT_LOADING_BAR;
    }
    if (hud.loading.LoadingBackground)
        hud.loading.LoadingBackground->ui_flags |= UIFLAG_EXTEND_WIDESCREEN_X;
}

void UI_WriteLoadingLayout(LPEDICT ent) {
    LPCMAPINFO info = level.mapinfo;
    LPCSTR title = info && info->loadingScreenTitle && *info->loadingScreenTitle ? info->loadingScreenTitle :
                   info ? info->mapName : NULL;
    LPCSTR background_key = info && info->loadingScreenModel && *info->loadingScreenModel
        ? info->loadingScreenModel : "LoadingMeleeBackground";
    LPCSTR background = background_key;
    /* W3I stores either a theme key (for example LoadingMeleeBackground) or
     * an actual map/import model such as Loading.mdx.  Only theme keys go
     * through Theme_PlayerString; resolving a filename as a theme key silently
     * replaced the map's own loading cover with the stock random screen. */
    if (!strchr(background_key, '\\') && !strchr(background_key, '/') &&
        !strstr(background_key, ".mdx") && !strstr(background_key, ".mdl")) {
        background = Theme_PlayerString(ent ? ent->client : NULL, background_key,
            "UI\\Glues\\Loading\\Multiplayer\\Load-Multiplayer-Random.mdx");
    }

    if (!ent || !hud.loading.Loading) return;
    if (hud.loading.LoadingTitleText)
        UI_SetText(hud.loading.LoadingTitleText, "%s", UI_LevelStringSafe(title));
    if (hud.loading.LoadingSubtitleText)
        UI_SetText(hud.loading.LoadingSubtitleText, "%s", UI_LevelStringSafe(info ? info->loadingScreenSubtitle : NULL));
    if (hud.loading.LoadingText)
        UI_SetText(hud.loading.LoadingText, "%s", UI_LevelStringSafe(info ? info->loadingScreenText : NULL));
    if (hud.loading.LoadingBackground)
        UI_SetPortraitFrameModel(hud.loading.LoadingBackground, gi.ModelIndex(background));
    UI_WriteLayout(ent, hud.loading.Loading, LAYER_LOADING);
}

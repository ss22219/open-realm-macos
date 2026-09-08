#define WC3_LEADERBOARD_MAX_ITEMS 64

typedef struct {
    char label[256];
    LONG value;
    LPPLAYER player;
    BOOL show_label, show_value, show_icon;
    COLOR32 label_color, value_color;
} wc3LeaderboardItem_t;

typedef struct {
    BOOL displayed;
    BOOL show_label, show_names, show_values, show_icons;
    char label[256];
    COLOR32 label_color, value_color;
    DWORD count;
    wc3LeaderboardItem_t items[WC3_LEADERBOARD_MAX_ITEMS];
} wc3Leaderboard_t;

static wc3Leaderboard_t *wc3_player_leaderboards[MAX_PLAYERS];

static wc3Leaderboard_t *WC3_Leaderboard(LPJASS j, int arg) {
    return jass_checkhandle(j, arg, "leaderboard");
}

static wc3LeaderboardItem_t *WC3_LeaderboardItem(wc3Leaderboard_t *lb, LONG index) {
    return lb && index >= 0 && (DWORD)index < lb->count ? lb->items + index : NULL;
}

static int WC3_LeaderboardCompareValue(const void *a, const void *b) {
    wc3LeaderboardItem_t const *x = a, *y = b;
    return (x->value > y->value) - (x->value < y->value);
}

static int WC3_LeaderboardCompareLabel(const void *a, const void *b) {
    wc3LeaderboardItem_t const *x = a, *y = b;
    return strcmp(x->label, y->label);
}

DWORD CreateLeaderboard(LPJASS j) {
    API_ALLOC(wc3Leaderboard_t, lb);
    lb->show_label = lb->show_names = lb->show_values = lb->show_icons = true;
    lb->label_color = MAKE(COLOR32, 255, 255, 255, 255);
    lb->value_color = lb->label_color;
    return 1;
}
DWORD DestroyLeaderboard(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) lb->displayed = false;
    return 0;
}
DWORD LeaderboardDisplay(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) lb->displayed = jass_checkboolean(j, 2);
    return 0;
}
DWORD IsLeaderboardDisplayed(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    return jass_pushboolean(j, lb && lb->displayed);
}
DWORD LeaderboardGetItemCount(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    return jass_pushinteger(j, lb ? (LONG)lb->count : 0);
}
DWORD LeaderboardSetSizeByItemCount(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    LONG count = jass_checkinteger(j, 2);
    if (lb) lb->count = MIN(MAX(count, 0), WC3_LEADERBOARD_MAX_ITEMS);
    return 0;
}
DWORD LeaderboardAddItem(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    LPPLAYER player = jass_checkhandle(j, 4, "player");
    if (lb && lb->count < WC3_LEADERBOARD_MAX_ITEMS) {
        wc3LeaderboardItem_t *item = lb->items + lb->count++;
        memset(item, 0, sizeof(*item));
        strlcpy(item->label, jass_checkstring(j, 2), sizeof(item->label));
        item->value = jass_checkinteger(j, 3);
        item->player = player;
        item->show_label = item->show_value = item->show_icon = true;
        item->label_color = item->value_color = MAKE(COLOR32, 255, 255, 255, 255);
    }
    return 0;
}
DWORD LeaderboardRemoveItem(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    LONG index = jass_checkinteger(j, 2);
    if (lb && index >= 0 && (DWORD)index < lb->count) {
        memmove(lb->items + index, lb->items + index + 1,
                (--lb->count - index) * sizeof(*lb->items));
    }
    return 0;
}
DWORD LeaderboardRemovePlayerItem(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    LPPLAYER player = jass_checkhandle(j, 2, "player");
    if (lb) FOR_LOOP(i, lb->count) {
        if (lb->items[i].player != player) continue;
        memmove(lb->items + i, lb->items + i + 1,
                (--lb->count - i) * sizeof(*lb->items));
        break;
    }
    return 0;
}
DWORD LeaderboardClear(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) lb->count = 0;
    return 0;
}
DWORD LeaderboardSortItemsByValue(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    BOOL ascending = jass_checkboolean(j, 2);
    if (lb) {
        qsort(lb->items, lb->count, sizeof(*lb->items), WC3_LeaderboardCompareValue);
        if (!ascending) FOR_LOOP(i, lb->count / 2) {
            wc3LeaderboardItem_t tmp = lb->items[i];
            lb->items[i] = lb->items[lb->count - i - 1];
            lb->items[lb->count - i - 1] = tmp;
        }
    }
    return 0;
}
DWORD LeaderboardSortItemsByPlayer(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    BOOL ascending = jass_checkboolean(j, 2);
    if (lb && lb->count > 1) {
        FOR_LOOP(i, lb->count - 1) FOR_LOOP(k, lb->count - i - 1) {
            DWORD a = lb->items[k].player ? PLAYER_NUM(lb->items[k].player) : MAX_PLAYERS;
            DWORD b = lb->items[k + 1].player ? PLAYER_NUM(lb->items[k + 1].player) : MAX_PLAYERS;
            if ((ascending && a > b) || (!ascending && a < b)) {
                wc3LeaderboardItem_t tmp = lb->items[k]; lb->items[k] = lb->items[k + 1]; lb->items[k + 1] = tmp;
            }
        }
    }
    return 0;
}
DWORD LeaderboardSortItemsByLabel(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    BOOL ascending = jass_checkboolean(j, 2);
    if (lb) {
        qsort(lb->items, lb->count, sizeof(*lb->items), WC3_LeaderboardCompareLabel);
        if (!ascending) FOR_LOOP(i, lb->count / 2) {
            wc3LeaderboardItem_t tmp = lb->items[i];
            lb->items[i] = lb->items[lb->count - i - 1];
            lb->items[lb->count - i - 1] = tmp;
        }
    }
    return 0;
}
DWORD LeaderboardHasPlayerItem(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    LPPLAYER player = jass_checkhandle(j, 2, "player");
    if (lb) FOR_LOOP(i, lb->count) if (lb->items[i].player == player) return jass_pushboolean(j, true);
    return jass_pushboolean(j, false);
}
DWORD LeaderboardGetPlayerIndex(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    LPPLAYER player = jass_checkhandle(j, 2, "player");
    if (lb) FOR_LOOP(i, lb->count) if (lb->items[i].player == player) return jass_pushinteger(j, i);
    return jass_pushinteger(j, -1);
}
DWORD LeaderboardSetLabel(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) strlcpy(lb->label, jass_checkstring(j, 2), sizeof(lb->label));
    return 0;
}
DWORD LeaderboardGetLabelText(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    return jass_pushstring(j, lb ? lb->label : "");
}
DWORD PlayerSetLeaderboard(LPJASS j) {
    LPPLAYER player = jass_checkhandle(j, 1, "player");
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 2);
    if (player && PLAYER_NUM(player) < MAX_PLAYERS) wc3_player_leaderboards[PLAYER_NUM(player)] = lb;
    return 0;
}
DWORD PlayerGetLeaderboard(LPJASS j) {
    LPPLAYER player = jass_checkhandle(j, 1, "player");
    return jass_pushlighthandle(j, player && PLAYER_NUM(player) < MAX_PLAYERS
        ? wc3_player_leaderboards[PLAYER_NUM(player)] : NULL, "leaderboard");
}
DWORD LeaderboardSetLabelColor(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) lb->label_color = MAKE(COLOR32, jass_checkinteger(j, 2), jass_checkinteger(j, 3), jass_checkinteger(j, 4), jass_checkinteger(j, 5));
    return 0;
}
DWORD LeaderboardSetValueColor(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) lb->value_color = MAKE(COLOR32, jass_checkinteger(j, 2), jass_checkinteger(j, 3), jass_checkinteger(j, 4), jass_checkinteger(j, 5));
    return 0;
}
DWORD LeaderboardSetStyle(LPJASS j) {
    wc3Leaderboard_t *lb = WC3_Leaderboard(j, 1);
    if (lb) { lb->show_label = jass_checkboolean(j, 2); lb->show_names = jass_checkboolean(j, 3); lb->show_values = jass_checkboolean(j, 4); lb->show_icons = jass_checkboolean(j, 5); }
    return 0;
}
DWORD LeaderboardSetItemValue(LPJASS j) {
    wc3LeaderboardItem_t *item = WC3_LeaderboardItem(WC3_Leaderboard(j, 1), jass_checkinteger(j, 2));
    if (item) item->value = jass_checkinteger(j, 3);
    return 0;
}
DWORD LeaderboardSetItemLabel(LPJASS j) {
    wc3LeaderboardItem_t *item = WC3_LeaderboardItem(WC3_Leaderboard(j, 1), jass_checkinteger(j, 2));
    if (item) strlcpy(item->label, jass_checkstring(j, 3), sizeof(item->label));
    return 0;
}
DWORD LeaderboardSetItemStyle(LPJASS j) {
    wc3LeaderboardItem_t *item = WC3_LeaderboardItem(WC3_Leaderboard(j, 1), jass_checkinteger(j, 2));
    if (item) { item->show_label = jass_checkboolean(j, 3); item->show_value = jass_checkboolean(j, 4); item->show_icon = jass_checkboolean(j, 5); }
    return 0;
}
DWORD LeaderboardSetItemLabelColor(LPJASS j) {
    wc3LeaderboardItem_t *item = WC3_LeaderboardItem(WC3_Leaderboard(j, 1), jass_checkinteger(j, 2));
    if (item) item->label_color = MAKE(COLOR32, jass_checkinteger(j, 3), jass_checkinteger(j, 4), jass_checkinteger(j, 5), jass_checkinteger(j, 6));
    return 0;
}
DWORD LeaderboardSetItemValueColor(LPJASS j) {
    wc3LeaderboardItem_t *item = WC3_LeaderboardItem(WC3_Leaderboard(j, 1), jass_checkinteger(j, 2));
    if (item) item->value_color = MAKE(COLOR32, jass_checkinteger(j, 3), jass_checkinteger(j, 4), jass_checkinteger(j, 5), jass_checkinteger(j, 6));
    return 0;
}

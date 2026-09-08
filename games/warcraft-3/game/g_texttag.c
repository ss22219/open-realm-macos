#include "g_local.h"
#include "jass/jass.h"

#define WC3_TEXTTAG_DEFAULT_HEIGHT 0.0276f
#define WC3_TEXTTAG_DEFAULT_LIFESPAN 600000u
#define WC3_TEXTTAG_PERMANENT_LIFESPAN 86400000u
#define WC3_TEXTTAG_TEXT_CAPACITY 256

typedef struct wc3TextTag_s {
    char text[WC3_TEXTTAG_TEXT_CAPACITY];
    VECTOR3 origin;
    FLOAT height;
    FLOAT velocity_x;
    FLOAT velocity_y;
    COLOR32 color;
    DWORD lifespan;
    DWORD fadepoint;
    DWORD age;
    BOOL positioned;
    BOOL visible;
    BOOL suspended;
    BOOL permanent;
    BOOL dirty;
} wc3TextTag_t;

typedef struct wc3TextTagNode_s {
    wc3TextTag_t *tag;
    struct wc3TextTagNode_s *next;
} wc3TextTagNode_t;

static wc3TextTagNode_t *wc3_text_tags;

static wc3TextTag_t *WC3_TextTag(LPJASS j, int index) {
    return (wc3TextTag_t *)jass_checkhandle(j, index, "texttag");
}

static void WC3_TextTagMarkDirty(wc3TextTag_t *tag) {
    if (tag) tag->dirty = true;
}

static DWORD WC3_TextTagFont(wc3TextTag_t *tag) {
    FLOAT height = tag && tag->height > 0.0f ? tag->height : WC3_TEXTTAG_DEFAULT_HEIGHT;
    DWORD size = (DWORD)MAX(8.0f, MIN(64.0f, height * 500.0f + 0.5f));
    return gi.FontIndex ? (DWORD)gi.FontIndex(Theme_String("MasterFont", "Fonts\\FRIZQT__.TTF"), size) : 0;
}

static void WC3_TextTagWrite(wc3TextTag_t *tag, LPEDICT recipient) {
    VECTOR3 origin;
    DWORD lifetime;
    DWORD fadepoint;
    DWORD font;
    LONG color;

    if (!tag || !tag->text[0] || !tag->positioned || !tag->visible || tag->suspended || !gi.Write)
        return;
    font = WC3_TextTagFont(tag);
    if (!font || font >= MAX_FONTSTYLES) return;
    lifetime = tag->lifespan ? tag->lifespan :
        (tag->permanent ? WC3_TEXTTAG_PERMANENT_LIFESPAN : WC3_TEXTTAG_DEFAULT_LIFESPAN);
    if (tag->age >= lifetime) return;
    lifetime -= tag->age;
    fadepoint = tag->fadepoint > tag->age ? tag->fadepoint - tag->age : 0;
    fadepoint = MIN(fadepoint, lifetime);
    origin = tag->origin;
    color = (LONG)tag->color.r | ((LONG)tag->color.g << 8) |
            ((LONG)tag->color.b << 16) | ((LONG)tag->color.a << 24);

    gi.Write(PF_BYTE, &(LONG){ svc_temp_entity });
    gi.Write(PF_BYTE, &(LONG){ TE_FLOATING_TEXT });
    gi.Write(PF_POSITION, &origin);
    gi.Write(PF_STRING, tag->text);
    gi.Write(PF_LONG, &color);
    gi.Write(PF_SHORT, &font);
    gi.Write(PF_LONG, &(LONG){ (LONG)lifetime });
    gi.Write(PF_LONG, &(LONG){ (LONG)fadepoint });
    gi.Write(PF_LONG, &(LONG){ (LONG)tag->age });
    gi.Write(PF_FLOAT, &tag->velocity_x);
    gi.Write(PF_FLOAT, &tag->velocity_y);
    if (recipient && gi.unicast) gi.unicast(recipient);
    else if (gi.multicast) gi.multicast(&origin, MULTICAST_ALL);
}

static void WC3_TextTagAdd(wc3TextTag_t *tag) {
    wc3TextTagNode_t *node = calloc(1, sizeof(*node));
    if (!node) return;
    node->tag = tag;
    node->next = wc3_text_tags;
    wc3_text_tags = node;
}

void G_TextTagReset(void) {
    wc3TextTagNode_t *node = wc3_text_tags;
    while (node) {
        wc3TextTagNode_t *next = node->next;
        free(node);
        node = next;
    }
    wc3_text_tags = NULL;
}

void G_TextTagFlush(void) {
    wc3TextTagNode_t *node;
    for (node = wc3_text_tags; node; node = node->next) {
        if (!node->tag || !node->tag->dirty) continue;
        WC3_TextTagWrite(node->tag, NULL);
        node->tag->dirty = false;
    }
}

void G_TextTagSendAll(LPEDICT client) {
    wc3TextTagNode_t *node;
    if (!client) return;
    for (node = wc3_text_tags; node; node = node->next)
        WC3_TextTagWrite(node->tag, client);
}

DWORD CreateTextTag(LPJASS j) {
    wc3TextTag_t *tag = jass_newhandle(j, sizeof(*tag), "texttag");
    if (!tag) return 0;
    tag->height = WC3_TEXTTAG_DEFAULT_HEIGHT;
    tag->color = COLOR32_WHITE;
    tag->visible = true;
    WC3_TextTagAdd(tag);
    return 1;
}

DWORD DestroyTextTag(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) tag->visible = false;
    return 0;
}

DWORD SetTextTagText(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    LPCSTR text = jass_checkstring(j, 2);
    if (tag) {
        snprintf(tag->text, sizeof(tag->text), "%s", G_LevelString(text ? text : ""));
        tag->height = jass_checknumber(j, 3);
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagPos(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) {
        tag->origin.x = jass_checknumber(j, 2);
        tag->origin.y = jass_checknumber(j, 3);
        tag->origin.z = CM_GetHeightAtPoint(tag->origin.x, tag->origin.y) +
                        jass_checknumber(j, 4) + 10.0f;
        tag->positioned = true;
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagPosUnit(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    LPEDICT unit = jass_checkhandle(j, 2, "unit");
    if (tag && unit) {
        tag->origin = unit->s.origin;
        tag->origin.z += jass_checknumber(j, 3) + 10.0f;
        tag->positioned = true;
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagColor(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) {
        tag->color = MAKE(COLOR32,
            jass_checkinteger(j, 2), jass_checkinteger(j, 3),
            jass_checkinteger(j, 4), jass_checkinteger(j, 5));
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagVelocity(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) {
        tag->velocity_x = jass_checknumber(j, 2);
        tag->velocity_y = jass_checknumber(j, 3);
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagVisibility(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) { tag->visible = jass_checkboolean(j, 2); WC3_TextTagMarkDirty(tag); }
    return 0;
}
DWORD SetTextTagSuspended(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) { tag->suspended = jass_checkboolean(j, 2); WC3_TextTagMarkDirty(tag); }
    return 0;
}
DWORD SetTextTagPermanent(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) { tag->permanent = jass_checkboolean(j, 2); WC3_TextTagMarkDirty(tag); }
    return 0;
}
DWORD SetTextTagLifespan(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) { tag->lifespan = (DWORD)MAX(0.0f, jass_checknumber(j, 2) * 1000.0f); WC3_TextTagMarkDirty(tag); }
    return 0;
}
DWORD SetTextTagFadepoint(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) { tag->fadepoint = (DWORD)MAX(0.0f, jass_checknumber(j, 2) * 1000.0f); WC3_TextTagMarkDirty(tag); }
    return 0;
}
DWORD SetTextTagAge(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) {
        tag->age = (DWORD)MAX(0.0f, jass_checknumber(j, 2) * 1000.0f);
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

/* A few maps call the Blizzard.j convenience layer from code paths that are
 * not present in the trimmed script bundle. Keep those calls native so the
 * actual text-tag state still reaches the renderer. */
DWORD SetTextTagTextBJ(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    LPCSTR text = jass_checkstring(j, 2);
    if (tag) {
        snprintf(tag->text, sizeof(tag->text), "%s", G_LevelString(text ? text : ""));
        tag->height = jass_checknumber(j, 3) * 0.023f / 10.0f;
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagColorBJ(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    if (tag) {
        FLOAT red = MAX(0.0f, MIN(100.0f, jass_checknumber(j, 2)));
        FLOAT green = MAX(0.0f, MIN(100.0f, jass_checknumber(j, 3)));
        FLOAT blue = MAX(0.0f, MIN(100.0f, jass_checknumber(j, 4)));
        FLOAT transparency = MAX(0.0f, MIN(100.0f, jass_checknumber(j, 5)));
        tag->color = MAKE(COLOR32, (BYTE)(red * 255.0f / 100.0f + 0.5f),
                          (BYTE)(green * 255.0f / 100.0f + 0.5f),
                          (BYTE)(blue * 255.0f / 100.0f + 0.5f),
                          (BYTE)((100.0f - transparency) * 255.0f / 100.0f + 0.5f));
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagVelocityBJ(LPJASS j) {
    wc3TextTag_t *tag = WC3_TextTag(j, 1);
    FLOAT speed = jass_checknumber(j, 2) * 0.071f / 128.0f;
    FLOAT angle = jass_checknumber(j, 3) * (FLOAT)(M_PI / 180.0);
    if (tag) {
        tag->velocity_x = speed * cosf(angle);
        tag->velocity_y = speed * sinf(angle);
        WC3_TextTagMarkDirty(tag);
    }
    return 0;
}

DWORD SetTextTagPermanentBJ(LPJASS j) { return SetTextTagPermanent(j); }
DWORD SetTextTagSuspendedBJ(LPJASS j) { return SetTextTagSuspended(j); }
DWORD SetTextTagLifespanBJ(LPJASS j) { return SetTextTagLifespan(j); }
DWORD SetTextTagFadepointBJ(LPJASS j) { return SetTextTagFadepoint(j); }
DWORD SetTextTagAgeBJ(LPJASS j) { return SetTextTagAge(j); }

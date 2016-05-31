#include <deadbeef/deadbeef.h>
#include <string.h>
#include <stdlib.h>

// #define trace(...) { fprintf(stderr, __VA_ARGS__); }

static DB_misc_t plugin;
static DB_functions_t *deadbeef;

DB_plugin_t *
lang_load (DB_functions_t *api) {
    deadbeef = api;
    return DB_PLUGIN (&plugin);
}

static int
lang_start (void) {
    return 0;
}

static int
lang_stop (void) {
    return 0;
}

// Removes lang tag when lang == -1, otherwise sets specified lang.
static int
lang_action_rate_helper (DB_plugin_action_t *action, int ctx, int lang)
{
    DB_playItem_t *it = NULL;
    ddb_playlist_t *plt = NULL;
    int num = 0;

    if (ctx == DDB_ACTION_CTX_SELECTION) {
        plt = deadbeef->plt_get_curr ();
        if (plt) {
            num = deadbeef->plt_getselcount (plt);
            it = deadbeef->plt_get_first (plt, PL_MAIN);
            while (it) {
                if (deadbeef->pl_is_selected (it)) {
                    break;
                }
                DB_playItem_t *next = deadbeef->pl_get_next (it, PL_MAIN);
                deadbeef->pl_item_unref (it);
                it = next;
            }
            deadbeef->plt_unref (plt);
        }
    } else if (ctx == DDB_ACTION_CTX_NOWPLAYING) {
        it = deadbeef->streamer_get_playing_track ();
        plt = deadbeef->plt_get_curr ();
        num = 1;
    }
    if (!it || !plt || num < 1) {
        goto out;
    }

    int count = 0;
    while (it) {
        if (deadbeef->pl_is_selected (it) || ctx == DDB_ACTION_CTX_NOWPLAYING) {
            deadbeef->pl_lock ();
            if (lang == -1) {
                deadbeef->pl_delete_meta(it, "lang");
            } else {
                deadbeef->pl_set_meta_int(it, "lang", lang);
            }
            
            ddb_event_track_t *ev = (ddb_event_track_t *)deadbeef->event_alloc(DB_EV_TRACKINFOCHANGED);
            ev->track = it;
            deadbeef->pl_item_ref(ev->track);
            deadbeef->event_send((ddb_event_t *)ev, 0, 0);

            const char *dec = deadbeef->pl_find_meta_raw (it, ":DECODER");
            char decoder_id[100];
            if (dec) {
                strncpy (decoder_id, dec, sizeof (decoder_id));
            }
            int match = it && dec;
            deadbeef->pl_unlock ();
            if (match) {
                int is_subtrack = deadbeef->pl_get_item_flags (it) & DDB_IS_SUBTRACK;
                if (is_subtrack) {
                    continue;
                }
                DB_decoder_t *dec = NULL;
                DB_decoder_t **decoders = deadbeef->plug_get_decoder_list ();
                for (int i = 0; decoders[i]; i++) {
                    if (!strcmp (decoders[i]->plugin.id, decoder_id)) {
                        dec = decoders[i];
                        if (dec->write_metadata) {
                            dec->write_metadata (it);
                        }
                        break;
                    }
                }
            }
            count++;
            if (count >= num) {
                break;
            }
        }
        DB_playItem_t *next = deadbeef->pl_get_next (it, PL_MAIN);
        deadbeef->pl_item_unref (it);
        it = next;
    }
    if (plt) {
        deadbeef->plt_modified (plt);
    }

out:
    if (it) {
        deadbeef->sendmessage(DB_EV_PLAYLISTCHANGED, 0, 0, 0);
        deadbeef->pl_item_unref (it);
    }
    return 0;
}

static int
lang_action_rate0 (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, 0);
}

static int
lang_action_rate1 (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, 1);
}

static int
lang_action_rate2 (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, 2);
}

static int
lang_action_rate3 (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, 3);
}

static int
lang_action_rate4 (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, 4);
}

static int
lang_action_rate5 (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, 5);
}

static int
lang_action_remove (DB_plugin_action_t *action, int ctx)
{
    return lang_action_rate_helper(action, ctx, -1);
}

static DB_plugin_action_t remove_lang_action = {
    .title = "Remove lang tag",
    .name = "lang_remove",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_remove,
    .next = NULL
};

static DB_plugin_action_t rate5_action = {
    .title = "Lang 5",
    .name = "lang_rate5",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_rate5,
    .next = &remove_lang_action
};

static DB_plugin_action_t rate4_action = {
    .title = "Lang 4",
    .name = "lang_rate4",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_rate4,
    .next = &rate5_action
};

static DB_plugin_action_t rate3_action = {
    .title = "Lang 3",
    .name = "lang_rate3",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_rate3,
    .next = &rate4_action
};

static DB_plugin_action_t rate2_action = {
    .title = "Lang 2",
    .name = "lang_rate2",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_rate2,
    .next = &rate3_action
};

static DB_plugin_action_t rate1_action = {
    .title = "Lang 1",
    .name = "lang_rate1",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_rate1,
    .next = &rate2_action
};

static DB_plugin_action_t rate0_action = {
    .title = "Lang 0",
    .name = "lang_rate0",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS | DB_ACTION_ADD_MENU,
    .callback2 = lang_action_rate0,
    .next = &rate1_action
};

static DB_plugin_action_t *
lang_get_actions (DB_playItem_t *it)
{
    return &rate0_action;
}

static DB_misc_t plugin = {
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 5,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.name = "lang",
    .plugin.descr = "Enables commands to rate song(s) by editing the metadata tag lang.",
    .plugin.copyright =
        "Rating plugin for DeaDBeeF Player\n"
        "Author: Christian Hernvall\n"
        "\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation; either version 2\n"
        "of the License, or (at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
    ,
    .plugin.website = "http://deadbeef.sf.net",
    .plugin.start = lang_start,
    .plugin.stop = lang_stop,
    .plugin.get_actions = lang_get_actions,
};
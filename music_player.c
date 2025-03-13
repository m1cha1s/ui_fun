#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>

#define BASE_ARENA
#define BASE_IMPLEMENTATION
#include "base.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#undef STB_DS_IMPLEMENTATION
#define IMPL
#include "ui.h"

Arena *per_song_arena = NULL;
Arena *temp_arena = NULL;

char *tprintf(char *fmt, ...) {
    va_list args;
    ssize buf_size;
    char *buf;

    va_start(args, fmt);
    buf_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    buf = arena_alloc(temp_arena, buf_size+1);

    va_start(args, fmt);
    vsnprintf(buf, buf_size+1, fmt, args);
    va_end(args);

    buf[buf_size] = 0;

    return buf;
}

int main() {
    SetWindowState(FLAG_WINDOW_RESIZABLE
                   | FLAG_WINDOW_HIGHDPI
                   | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(800, 600, "music player");
    InitAudioDevice();
    per_song_arena = arena_new();
    temp_arena = arena_new();
    ui_state = ui_init();

    Font f = LoadFont("LiberationMono-Regular.ttf");
    ui_state->font = f;

    ui_state->root_node->dim.xy[0] = 0;
    ui_state->root_node->dim.xy[1] = 0;

    ui_state->root_node->dim.wh[0] = GetScreenWidth();
    ui_state->root_node->dim.wh[1] = GetScreenHeight();

    UI_Node *p = NULL;
    int playing = 0;
    int recursive = 0;
    char *current_song_path = NULL;
    FilePathList fp = {0};
    Music current_music = {0};

    float vol = 1.0f;

    while (!WindowShouldClose()) {
        UpdateMusicStream(current_music);

        ui_build_begin();

        ui_state->root_node->dim.wh[0] = GetScreenWidth();
        ui_state->root_node->dim.wh[1] = GetScreenHeight();

        p = ui_h_panel(S("file path"), UI_DRAW_BORDER);
        p->size[0].kind = UI_Size_Parent_Percent;
        p->size[0].value = 1;
        p->size[1].kind = UI_Size_Parent_Percent;
        p->size[1].value = 0.1;
        ui_push_parent(p);
        {
            ui_label(S("music extension:"), 0);
            u8 *ext = ui_text_input(S("file ext"), 0);
            if (!ext) ext = ".mp3";

            ui_label(S("music folder:"), 0);
            u8 *path = ui_text_input(S("file path text box"), 0);
            if (path) {
                if (DirectoryExists(path)) {
                    if (fp.capacity) UnloadDirectoryFiles(fp);
                    fp = LoadDirectoryFilesEx(path, ext, recursive);
                }
            }

            ui_label(S("recursive search"), 0);
            String rs = recursive ? S("X") : S("O");
            if (ui_button(rs, 0)) {
                recursive = !recursive;
            }

            if (ui_button(S("-"), 0)) { if (vol > 0) vol -= 0.1; SetMasterVolume(vol); }
            u8 *vol_txt = tprintf("%d%%", (int)(100*vol));
            String vol_str = {vol_txt, strlen(vol_txt)};
            ui_label(vol_str, 0);
            if (ui_button(S("+"), 0)) { if (vol < 1) vol += 0.1; SetMasterVolume(vol); }
        }
        ui_pop_parent();

        p = ui_v_panel(S("files list"), UI_DRAW_BORDER | UI_SCROLLABLE);
        p->size[0].kind = UI_Size_Parent_Percent;
        p->size[0].value = 1;
        p->size[1].kind = UI_Size_Parent_Percent;
        p->size[1].value = 0.8;
        ui_push_parent(p);
        {
            for (int i = 0; i < fp.count; ++i) {
                String bs = {.str=fp.paths[i], .len=strlen(fp.paths[i])};
                if (ui_button(bs, 0)) {
                    printf("%s\n", fp.paths[i]);

                    if (IsMusicValid(current_music)) UnloadMusicStream(current_music);
                    arena_reset(per_song_arena);
                    current_song_path = aprintf(per_song_arena, "%s", fp.paths[i]);
                    current_music = LoadMusicStream(fp.paths[i]);
                    PlayMusicStream(current_music);
                }
            }
        }
        ui_pop_parent();

        p = ui_h_panel(S("controls"), UI_DRAW_BORDER);
        p->size[0].kind = UI_Size_Parent_Percent;
        p->size[0].value = 1;
        p->size[1].kind = UI_Size_Parent_Percent;
        p->size[1].value = 0.1;
        ui_push_parent(p);
        {
            if (ui_button(S("<<"), 0)) {
                printf("Prev\n");
            }
            String pp = !IsMusicStreamPlaying(current_music) ? S("||") : S(">");
            if (ui_button(pp, 0)) {
                printf("play/pause\n");
                if (IsMusicStreamPlaying(current_music)) PauseMusicStream(current_music);
                else ResumeMusicStream(current_music);
            }
            if (ui_button(S(">>"), 0)) {
                printf("next\n");
            }

            if (current_song_path) {

                float time_played = GetMusicTimePlayed(current_music)/GetMusicTimeLength(current_music);
                int prog = time_played*10;
                char *progress_msg = tprintf("[%*.*s]", prog-10, prog, "==========");
                String ps = {progress_msg, strlen(progress_msg)};
                ui_label(ps, 0);

                char *status_msg = tprintf("Now playing: %s\n", GetFileNameWithoutExt(current_song_path));
                String sm = {status_msg, strlen(status_msg)};
                ui_label(sm, 0);
            }
        }
        ui_pop_parent();

        BeginDrawing();

        ClearBackground(BLACK);
        ui_build_end();

        EndDrawing();

        arena_reset(temp_arena);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

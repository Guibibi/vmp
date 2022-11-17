#ifndef analyzer_h
#define analyzer_h

#include "mt_vector.c"
#include <linux/limits.h>
#include <stdlib.h>

typedef struct _analyzer_t
{
    mt_vector_t* songs;
    float        ratio;
} analyzer_t;

analyzer_t* analyzer_run(mt_vector_t* songs);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "coder.c"
#include "config.c"
#include "mt_map.c"
#include "mt_path.c"
#include "mt_string.c"
#include <SDL.h>
#include <time.h>

int analyzer_thread(void* chptr)
{
    analyzer_t*  analyzer = chptr;
    mt_vector_t* trash    = VNEW();
    char*        libpath  = config_get("lib_path");

    for (int index = 0; index < analyzer->songs->length; index++)
    {
	mt_map_t* song     = analyzer->songs->data[index];
	char*     path     = MGET(song, "path");
	char*     time_str = CAL(80, NULL, mt_string_describe); // REL 0

	time_t now;
	time(&now);
	struct tm ts = *localtime(&now);
	strftime(time_str, 80, "%Y-%m-%d %H:%M", &ts);

	// add file data

	MPUT(song, "added", time_str);
	MPUTR(song, "played", STRNC("0"));
	MPUTR(song, "skipped", STRNC("0"));
	MPUTR(song, "plays", STRNC("0"));
	MPUTR(song, "skips", STRNC("0"));

	char* real = STRNF(PATH_MAX + NAME_MAX, "%s/%s", libpath, path); // REL 1

	// read and add file and meta data

	int res = coder_load_metadata_into(real, song);

	if (res == 0)
	{
	    char* artist = MGET(song, "artist");
	    char* album  = MGET(song, "album");
	    char* title  = MGET(song, "title");
	    if (strcmp(artist, "...") == 0) MPUTR(song, "artist", STRNC("Unknown"));
	    if (strcmp(album, "...") == 0) MPUTR(song, "album", STRNC("Unknown"));
	    if (strcmp(title, "...") == 0) MPUTR(song, "title", mt_path_new_filename(path));
	}
	else
	{
	    // file is not a media file readable by ffmpeg, we skip it
	    VADD(trash, song);
	}

	// cleanup
	REL(real);     // REL 1
	REL(time_str); // REL 0

	// show progress
	analyzer->ratio = (float) index / (float) analyzer->songs->length;
    }

    mt_vector_rem_in_vector(analyzer->songs, trash);

    REL(trash);

    analyzer->ratio = 1.0;

    return 0;
}

void analyzer_del(void* pointer)
{
    analyzer_t* analyzer = pointer;

    REL(analyzer->songs);
}

analyzer_t* analyzer_run(mt_vector_t* songs)
{
    analyzer_t* analyzer = CAL(sizeof(analyzer_t), analyzer_del, NULL);

    analyzer->songs = RET(songs);

    SDL_CreateThread(analyzer_thread, "analyzer", analyzer);

    return analyzer;
}

#endif

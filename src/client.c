// itv-libraries: sdl2 SDL2_image json-c

#include "common.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <limits.h>

int
main(void)
{
	bool			ok		= false;
	bool			go		= true;
	uint			window_flags	= SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
	uint			renderer_flags	= SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	uint			img_flags	= IMG_INIT_JPG | IMG_INIT_PNG;
	SDL_Rect		disp		= {0};
	SDL_Window *		window		= NULL;
	SDL_Renderer *		renderer	= NULL;
	char const *		texture_name	= 0;
	SDL_Texture *		texture		= NULL;
	int			texture_w	= 0;
	int			texture_h	= 0;
	char const *		content_dir	= "./content";
	static char		path[4096]	= {0};
	time_t			utc		= time(0);
	time_t			switch_time	= 0;
	struct tm		tm		= {0};
	struct itv_schedule	schedule	= {0};
	SDL_Event		event		= {0};

	itv_log_register(itv_log_file, stderr);

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		itv_fail("SDL_Init(): %s", SDL_GetError());

	snprintf(path, sizeof path, "%s/_schedule.json", content_dir);
	if (itv_schedule_load(&schedule, path))
		goto fail;

	switch_time = utc + schedule.slot_seconds;

	if (SDL_GetDisplayBounds(0, &disp) != 0)
		disp = (SDL_Rect){0, 0, 1920, 1080};
	if (!(window = SDL_CreateWindow("infotv-client", disp.x, disp.y, disp.w, disp.h, window_flags)))
		itv_fail("SDL_CreateWindow(): %s", SDL_GetError());
	if (!(renderer = SDL_CreateRenderer(window, -1, renderer_flags)))
		itv_fail("SDL_CreateRenderer(): %s", SDL_GetError());
	if ((IMG_Init(img_flags) & img_flags) != img_flags)
		itv_warn("IMG_Init(): Couldn't load all image libraries.");

	texture_name = itv_schedule_next(&schedule, localtime_r(&utc, &tm));
	snprintf(path, sizeof path, "%s/%s", content_dir, texture_name);
	if (!(texture = IMG_LoadTexture(renderer, path)))
		itv_fail("IMG_LoadTexture(%s): %s", path, IMG_GetError());
	if (SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h))
		itv_fail("SDL_QueryTexture(%s): %s", path, IMG_GetError());

	while (go) {
		utc = time(0);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: go = false; break;
			}
		}

		if (switch_time <= utc) {
			SDL_DestroyTexture(texture);
			texture_name = itv_schedule_next(&schedule, localtime_r(&utc, &tm));
			snprintf(path, sizeof path, "%s/%s", content_dir, texture_name);
			if (!(texture = IMG_LoadTexture(renderer, path)))
				itv_fail("IMG_LoadTexture(%s): %s", path, IMG_GetError());
			if (SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h))
				itv_fail("SDL_QueryTexture(%s): %s", path, IMG_GetError());
			switch_time += schedule.slot_seconds;
		}

		int dst_w = texture_w, dst_h = texture_h;

		int diff_w = dst_w - disp.w, diff_h = dst_h - disp.h;
		if (diff_w > 0 || diff_h > 0) {
			if (diff_w > diff_h) {
				dst_h = (disp.w*dst_h) / dst_w;
				dst_w = disp.w;
			} else {
				dst_w = (disp.h*dst_w) / dst_h;
				dst_h = disp.h;
			}
		}

		SDL_Rect src = {0, 0, texture_w, texture_h};
		SDL_Rect dst = {
			(disp.w - dst_w) / 2,
			(disp.h - dst_h) / 2,
			dst_w,
			dst_h
		};

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, &src, &dst);
		SDL_RenderPresent(renderer);
	}

	ok = true;
fail:
	SDL_DestroyTexture(texture);
	IMG_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

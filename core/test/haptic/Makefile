OUT = ff_lg_test haptic_waveform_test haptic_stats_test haptic_rate_sim
OBJS = ../../haptic/common/ff_lg.o ../../haptic/haptic_tweaks.o
WOBJS = $(OBJS) ../../haptic/haptic_waveform.o ../../haptic/haptic_synth.o ../../haptic/haptic_source.o ../../haptic/source/haptic_source_lg.o
BINS = ff_lg_test haptic_waveform_test haptic_stats_test haptic_rate_sim
CFLAGS = -I../../ -I../../../shared -I../../../shared -Wall -Wextra -Werror -g -O0
CXXFLAGS = -Wall -Wextra -Werror -g -O0

ifeq ($(OS),Windows_NT)
CPPFLAGS += $(shell sdl2-config --cflags)
CPPFLAGS := $(filter-out -Dmain=SDL_main,$(CPPFLAGS))
LDLIBS = -lintl
endif

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o ../../haptic/haptic_waveform.o ../../haptic/haptic_synth.o ../../haptic/haptic_source.o ../../haptic/source/haptic_source_lg.o

ff_lg_test: $(OBJS)

haptic_waveform_test: $(WOBJS)

haptic_stats_test: ../../haptic/haptic_stats.o

haptic_rate_sim: $(OBJS) ../../haptic/haptic_waveform.o ../../haptic/haptic_synth.o

test: all
	./ff_lg_test > /dev/null
	./haptic_waveform_test
	./haptic_stats_test

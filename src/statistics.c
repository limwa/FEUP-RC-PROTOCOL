#include "statistics.h"

#include <time.h>
#include <sys/time.h>

static struct timeval start;
static long received_bits;
static long bad_frames, good_frames;


void statistics_start_transfer() {
   gettimeofday(&start, NULL);
}

double statistics_get_transfer_time() {
    struct timeval curr;
    gettimeofday(&curr, NULL);

    return (curr.tv_sec - start.tv_sec) + (curr.tv_usec - start.tv_usec) / 1000000.0;
}


void statistics_add_received_bytes(long bytes) {
    received_bits += bytes << 3; // bytes * 8
}

double statistics_get_received_bitrate(double transfer_time) {
    return received_bits / transfer_time;
}

void statistics_count_frame_bad() {
    bad_frames++;
}

void statistics_count_frame_good() {
    good_frames++;
}

double statistics_get_fer() {
    printf("bad: %ld\n", bad_frames);
    printf("good: %ld\n", good_frames);
    return bad_frames / ((double) (bad_frames + good_frames));
}

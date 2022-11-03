#include "statistics.h"

#include <time.h>
#include <sys/time.h>

static struct timeval start;
static long received_bits, last_received_bytes;
static long bad_frames, good_frames;


void statistics_start_transfer() {
   gettimeofday(&start, NULL);
}

double statistics_get_transfer_time() {
    struct timeval curr;
    gettimeofday(&curr, NULL);

    return (curr.tv_sec - start.tv_sec) + (curr.tv_usec - start.tv_usec) / 1000000.0;
}

void statistics_set_last_received_bytes(long bytes) {
    last_received_bytes = bytes;
}

void statistics_add_received_bytes() {
    received_bits += last_received_bytes << 3; // bytes * 8
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
    return bad_frames / ((double) (bad_frames + good_frames));
}

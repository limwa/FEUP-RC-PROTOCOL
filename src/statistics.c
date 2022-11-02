#include "statistics.h"

static clock_t start;
static long received_bits;
static long bad_frames, good_frames;


void statistics_start_transfer() {
    start = clock();
}

double statistics_get_transfer_time() {
    clock_t curr = clock();
    return (curr - start) / ((double) CLOCKS_PER_SEC);
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
    return bad_frames / ((double) (bad_frames + good_frames));
}

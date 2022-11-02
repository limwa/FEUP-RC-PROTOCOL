#include <time.h>

void statistics_start_transfer();
double statistics_get_transfer_time();

void statistics_add_received_bytes(long bytes);
double statistics_get_received_bitrate(double transfer_time);

void statistics_count_frame_bad();
void statistics_count_frame_good();
double statistics_get_fer();

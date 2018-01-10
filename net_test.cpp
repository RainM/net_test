#include "client.hpp"
#include "common.hpp"

#include <hdr_histogram.h>

#include <iomanip>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

#include <getopt.h>
#include <unistd.h>

#define ITERATIONS_IN_TEST 10

int silent_mode = 0;
int print_help = 0;

static struct option net_test_options[] = {
    { "silent", no_argument, &silent_mode, 1},
    { "time-to-run", required_argument, 0, 10},
    { "write-histogram-file-name", required_argument, 0, 11},
    { "max-write-histogram", required_argument, 0, 12},
    { "wait-histogram-file-name",  required_argument, 0, 13},
    { "max-wait-histogram",  required_argument, 0, 14},
    { "help", no_argument, &print_help, 15},
    { 0, 0, 0, 0}
};

FILE* try_open_file(const std::string& path) {
    FILE* result = fopen(path.c_str(), "wb");
    if (result == 0) {
	__THROW_EXCEPTION_WITH_LOCATION("Can't open file " << path << " for write");
    }
    return result;
}

int main(int argc, char** argv) {
    struct hdr_histogram* histogram_write;
    struct hdr_histogram* histogram_wait;

    if (argc < 3) {
	std::cerr << "No enough parameters\n";
	std::cerr << argv[0] << " host port [additional arguments]" << std::endl;
	return -1;
    }

    std::string host = argv[1];
    std::string port = argv[2];

    int time_to_run = 10;

    unique_file write_histogram_file, wait_histogram_file;

    int64_t max_wait_histogram = 1000000000L; // 1s
    int64_t max_write_histogram = 100000000L; // 100ms

    int c;
    int idx;
    while ( (c = ::getopt_long(argc, argv, "", net_test_options, &idx)) != -1 ) {
	switch (c) {
	case 0:
	    break;
	case 10:
	    time_to_run = ::atoi(optarg);
	    break;
	case 11:
	    write_histogram_file.reset(try_open_file(optarg));
	    break;
	case 13:
	    wait_histogram_file.reset(try_open_file(optarg));
	    break;
	case 12:
	    max_write_histogram = ::atol(optarg);
	    break;
	case 14:
	    max_wait_histogram = ::atol(optarg);
	    break;
	default:
	    return -1;
	}
    }

    if (print_help) {
	std::cerr << "net_nest" << std::endl;
	std::cerr << "Usage: " << argv[0] << " host port [additional args]" << std::endl;
	std::cerr << "Additional arguments" << std::endl;
	std::cerr << "  --silent                                   - enable silent mode. disabled by default" << std::endl;
	std::cerr << "  --time-to-run TIME_IN_SEC                  - time to measure. Default 10sec" << std::endl;
	std::cerr << "  --write-histogram-file-name FILE_NAME      - if specified, outputs write histogram to this file" << std::endl;
	std::cerr << "  --write-histogram-max MAX_HISTOGRAM_VALUE  - max value for write histogram. Default 100ms" << std::endl;
	std::cerr << "  --wait-histogram-file-name FILE_NAME       - if specified, outputs wait histogram to this file" << std::endl;
	std::cerr << "  --wait-histogram-max MAX_HISTOGRAM_VALUE   - max value for wait histogram. Default 1sec" << std::endl;
	std::cerr << "  --help                                     - this message" << std::endl;
	return 1;
    }

    try {
	if (::hdr_init(
	    1,
	    max_write_histogram,
	    3,
	    &histogram_write) != 0)
	{

	    __THROW_EXCEPTION_WITH_LOCATION("Can't create WRITE histogram");
	}

	if (::hdr_init(
	    1000,
	    max_wait_histogram,
	    3,
	    &histogram_wait) != 0) {
	    __THROW_EXCEPTION_WITH_LOCATION("Can't create WAIT histogram");
	}


	client clt(host, port);

	double total_delay_ns = 0. ;
	for (int i = 0; i < ITERATIONS_IN_TEST; i++) {
	    auto delay = clt.ping_pong();
	    total_delay_ns += delay.first + delay.second;
	}

	int64_t iterations_in_1_sec = (BILLION_F / total_delay_ns) * ITERATIONS_IN_TEST;
	int64_t wake_each_iterations = iterations_in_1_sec / 5;

	auto start_point = std::chrono::system_clock::now();
	auto end_point = start_point + std::chrono::seconds(time_to_run);
	auto next_wake_up = start_point + std::chrono::seconds(1);

	while (end_point > std::chrono::system_clock::now()) {
	    for (int64_t i = 0; i < wake_each_iterations; ++i) {
		auto delay = clt.ping_pong();
		hdr_record_value(
		    histogram_write,
		    delay.first);
		hdr_record_value(
		    histogram_wait,
		    delay.second);
	    }

	    if (std::chrono::system_clock::now() > next_wake_up) {
		next_wake_up = std::chrono::system_clock::now() + std::chrono::seconds(1);
	    }
	}

	if (!silent_mode) {
	    std::cout << "----------------------------------------" << std::endl;
	    std::cout << "--------------------Write----------------" << std::endl;
	    hdr_percentiles_print(
		histogram_write,
		stdout,
		1,
		1.0,
		CLASSIC);

	    std::cout << "--------------------Wait----------------" << std::endl;
	    hdr_percentiles_print(
		histogram_wait,
		stdout,
		1,
		1.0,
		CLASSIC);
	    std::cout << "----------------------------------------" << std::endl;
	}

	if (write_histogram_file) {
	    hdr_percentiles_print(
		histogram_write,
		write_histogram_file.get(),
		1,
		1.0,
		CLASSIC);
	}
	if (wait_histogram_file) {
	    hdr_percentiles_print(
		histogram_wait,
		wait_histogram_file.get(),
		1,
		1.0,
		CLASSIC);
	}

    } catch (std::exception& e) {
	std::cerr << "Fatal error" << std::endl;
	std::cerr << e.what() << std::endl;
	return 2;
    }

    return 0;
}

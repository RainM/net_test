#include "client.hpp"
#include "common.hpp"

#include <hdr_histogram.h>

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

#include <unistd.h>

#define ITERATIONS_IN_TEST 10

int main(int argc, char** argv) {    
    std::cout << "Hello, world!" << std::endl;
				    
    struct hdr_histogram* histogram_write;
    struct hdr_histogram* histogram_wait;

    ::hdr_init(
	       1,  // Minimum value
	       INT64_C(3600000000),  // Maximum value
	       3,  // Number of significant figures
	       &histogram_write);  // Pointer to initialise
    ::hdr_init(
	1,  // Minimum value
	INT64_C(3600000000),  // Maximum value
	3,  // Number of significant figures
	&histogram_wait);  // Pointer to initialise
    
    if (argc < 3) {
	std::cerr << "No enough parameters\n";
	std::cerr << argv[0] << " host port" << std::endl;
	return -1;
    }

    int print_histograms_each_n_seconds = 1;
    int time_to_run = 10;
    std::vector<double> percentiles_to_output;
    
    int c;
    while ( (c = ::getopt(argc, argv, "p:n:t:e")) != -1 ) {
	switch (c) {
	case 'p':
	    percentiles_to_output.push_back(::atof(optarg));
	    break;
	case 'n':
	    print_histograms_each_n_seconds = ::atoi(optarg);
	    break;
	case 't':
	    time_to_run = ::atoi(optarg);
	    break;
	case 'e':
	    time_to_run = -1;
	default:
	    return -1;
	}
    }

    if (percentiles_to_output.empty()) {
	percentiles_to_output.push_back(0.50);
	percentiles_to_output.push_back(0.99);
    }
    
    try {
    
	std::string host = argv[1];
	int port = atoi(argv[2]);
    
	client clt(host, port);

	double total_delay_ns = 0. ;
	for (int i = 0; i < ITERATIONS_IN_TEST; i++) {
	    auto delay = clt.ping_pong();
	    total_delay_ns += delay.first + delay.second;
	}

	int64_t iterations_in_1_sec = (BILLION_F / total_delay_ns) * ITERATIONS_IN_TEST; // 97
	int64_t wake_each_iterations = iterations_in_1_sec / 5;


	auto start_point = std::chrono::system_clock::now();
	auto end_point = start_point + std::chrono::seconds(time_to_run);
	auto next_wake_up = start_point + std::chrono::seconds(print_histograms_each_n_seconds);

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
		next_wake_up = std::chrono::system_clock::now() + std::chrono::seconds(print_histograms_each_n_seconds);
		
		std::for_each(
		    std::begin(percentiles_to_output),
		    std::end(percentiles_to_output),
		    [histogram_write] ( const double& percentile) {
			int64_t write = ::hdr_value_at_percentile(histogram_write, percentile);
			std::cout << "Write " << percentile << ": " << write << std::endl;
		    }
		    );
		std::for_each(
		    std::begin(percentiles_to_output),
		    std::end(percentiles_to_output),
		    [histogram_wait] ( const double& percentile) {
			int64_t wait = ::hdr_value_at_percentile(histogram_wait, percentile);
			std::cout << "Wait " << percentile << ": " << wait << std::endl;
		    }
		    );
	    }
	}


	std::cout << "Write:" << std::endl;
	hdr_percentiles_print(
	    histogram_write,
	    stdout,
	    1,
	    1.0,
	    CLASSIC);
	
	std::cout << "Wait:" << std::endl;
	hdr_percentiles_print(
	    histogram_wait,
	    stdout,
	    1,
	    1.0,
	    CLASSIC);
	
    } catch (std::exception& e) {
	std::cerr << "Fatal error" << std::endl;
	std::cerr << e.what() << std::endl;
	::exit (-1);
    }

    /*
    hdr_percentiles_print(
	histogram,
	stdout,  // File to write to
	1,  // Granularity of printed values
	1.0,  // Multiplier for results
	CLASSIC);  // Format CLASSIC/CSV supported.

    */
}

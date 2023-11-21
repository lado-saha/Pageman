"""
Pageman Profiler Documentation
By LADO SAHA


The Pageman Profiler is a tool that analyzes the performance of the Pageman algorithm using system logs. It calculates statistics to evaluate memory savings and time overhead.

Statistics Calculated
---------------------
The Pageman Profiler calculates:
- Fit Interception Count: Number of fit interceptions in logs.
- Free Interception Count: Number of free interceptions in logs.
- Required Pages: Pages required for each fit interception.
- Original Pages: Original number of pages before fit interception.
- Elapsed Time: Time elapsed during each fit interception.
- Elapsed Time: Time elapsed during each free interception.
- Overall Percentage Saved: Percentage of memory saved by Pageman.
- Total Number of Pages Saved: Total number of pages saved.
- Total Memory Saved: Total memory saved in megabytes.
- Total Time Elapsed at Allocation: Total time elapsed during fit interceptions.
- Total Time Elapsed at Free: Total time elapsed during free interceptions.

"""

import re
import subprocess

def evaluate_algorithm(log_file_path):
    saved_pages = []                 # List to store the number of saved pages for each fit interception
    original_pages = []              # List to store the number of original pages for each fit interception
    time_elapsed = []                # List to store the time elapsed for each fit interception
    time_elapsed_on_free = []        # List to store the time elapsed for each free interception

    command = ['journalctl', '-b']   # Command to retrieve the system logs
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, error = process.communicate()

    # Decode the output as a string
    logs = output.decode('utf-8')

    for line in logs.split("\n"):
        match_fit = re.search(r"Stats\|\s*Fit_size=(\d+)\s+KB\s+Required_pages=(\d+)\s+Original_pages=(\d+)\s+Elapse=(\d+)\s+ns", line)
        match_free = re.search(r"Stats\|\s*Free_size=(\d+)\s+KB\s+fit_pages=(\d+)\s+Max_pages=(\d+)\s+Elapse=(\d+)\s+ns", line)

        if match_fit:
            required_pages = int(match_fit.group(2))
            original = int(match_fit.group(3))
            time_ns = int(match_fit.group(4))
            saved_pages.append(original - required_pages)
            original_pages.append(original)
            time_elapsed.append(time_ns / (1e9))  # Convert nanoseconds to seconds

        elif match_free:
            time_ns = int(match_free.group(4))
            time_elapsed_on_free.append(time_ns / 1e9)

    # Calculate overall statistics
    total_saved_pages = sum(saved_pages)
    total_original_pages = sum(original_pages)
    total_time_elapsed = sum(time_elapsed)
    total_time_elapsed_free = sum(time_elapsed_on_free)

    percentage_saved = ((total_original_pages - total_saved_pages) / total_original_pages) * 100
    memory_saved_kb = total_saved_pages * 4  # Assuming 4 KB per page

    # Run uptime command to get system uptime
    command = ['uptime', '-p']
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, error = process.communicate()

    # Decode the output as a string
    uptime_info = output.decode('utf-8').strip()

    # Print report
    print("Pageman Report (By LADO SAHA)")
    print(f"uptime: {uptime_info}")
    print("------")
    print(f"Fit Interception count: {len(saved_pages)}")
    print(f"Free Interception count: {len(time_elapsed_on_free)}\n")
    print(f"Overall Percentage Saved: {percentage_saved:.7f}%")
    print(f"Total Number of Pages Saved: {total_saved_pages}")
    print(f"Total Memory Saved: {memory_saved_kb / 1024} MB\n")
    print(f"Total Time Elapsed at allocation: {total_time_elapsed:.2f} seconds")
    print(f"Total Time Elapsed at Free: {total_time_elapsed_free:.2f} seconds")

# Example usage
log_file_path = "/var/log/kern.log"  # Path to the kernel log file, adjust as needed
evaluate_algorithm(log_file_path)

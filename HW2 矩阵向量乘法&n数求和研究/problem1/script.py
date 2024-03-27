import csv
import subprocess
import re
import matplotlib.pyplot as plt


program_names = ["matrix", "matrix_unroll", "matrix_template"]

save_csv = True
save_fig = True

for program_name in program_names:
    # 存储数据
    n_values = []
    no_cache_avg_times = []
    cache_friendly_avg_times = []

    if (program_name == "matrix" or program_name == "matrix_unroll"):
        min_N = 32
        max_N = 2048
        step = 32
    elif (program_name == "matrix_template"):
        min_N = 2
        max_N = 128
        step = 2


    # 运行命令并捕获输出
    for N in range(min_N, max_N + 1, step):
        print(f"Running: make {program_name} N={N} for 3 iterations")

        # 临时存储每次运行的时间
        temp_no_cache_times = []
        temp_cache_friendly_times = []
        result = subprocess.run(
                f"make {program_name} N={N}", shell=True, capture_output=True, text=True)
            
        for _ in range(3):  # 对每个N运行三次
            result = subprocess.run(
                f"./{program_name}_{N}", shell=True, capture_output=True, text=True)

            output = result.stdout

            # 解析输出以提取时间
            no_cache_match = re.search(
                "no-cache computation: (\d+) microseconds", output)
            cache_friendly_match = re.search(
                "cache-friendly computation: (\d+) microseconds", output)

            if no_cache_match and cache_friendly_match:
                temp_no_cache_times.append(int(no_cache_match.group(1)))
                temp_cache_friendly_times.append(
                    int(cache_friendly_match.group(1)))

        # 计算平均时间并存储
        if temp_no_cache_times and temp_cache_friendly_times:
            avg_no_cache_time = sum(temp_no_cache_times) / len(temp_no_cache_times)
            avg_cache_friendly_time = sum(
                temp_cache_friendly_times) / len(temp_cache_friendly_times)

            n_values.append(N)
            no_cache_avg_times.append(avg_no_cache_time)
            cache_friendly_avg_times.append(avg_cache_friendly_time)
    # 绘制图表
    plt.figure(figsize=(10, 6))
    plt.plot(n_values, no_cache_avg_times, 'o-', label='Average No-Cache Computation Time', markersize=4)
    plt.plot(n_values, cache_friendly_avg_times, 'o-', label='Average Cache-Friendly Computation Time', markersize=4)
    plt.xlabel('Matrix Size N')
    plt.ylabel('Time (microseconds)')
    plt.title(f'Average Computation Time vs. Matrix Size for {program_name}')
    plt.legend()
    plt.grid(True)
    if save_fig:
        plt.savefig(f'computation_time_vs_matrix_size_{program_name}_{step}.png')
    plt.close()


    # save no_cache_avg_times and cache_friendly_avg_times to a csv file
    if save_csv:
        with open(f'result_{program_name}.csv', 'w', newline='') as csvfile:
            # if csv exsits, clean it
            csvfile.truncate()
            writer = csv.writer(csvfile)
            writer.writerow(['N', 'No-Cache Avg Time', 'Cache-Friendly Avg Time'])
            for n, no_cache, cache_friendly in zip(n_values, no_cache_avg_times, cache_friendly_avg_times):
                writer.writerow([n, no_cache, cache_friendly])


    # 清理生成的文件
    subprocess.run("make clean", shell=True)

# 计算有无unroll的加速比
with open('result_matrix.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # skip header
    results = list(reader)

    no_cache_times = {}
    cache_friendly_times = {}
    for row in results:
        N = int(row[0])
        no_cache_times[N] = float(row[1])
        cache_friendly_times[N] = float(row[2])


with open('result_matrix_unroll.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # skip header
    results = list(reader)

    no_cache_times_unroll = {}
    cache_friendly_times_unroll = {}
    for row in results:
        N = int(row[0])
        no_cache_times_unroll[N] = float(row[1])
        cache_friendly_times_unroll[N] = float(row[2])
        
# do element-wise: (cache_friendly_times - cache_friendly_times_unroll) / cache_friendly_times
unroll_speedup_cache_friendly = {k: (cache_friendly_times[k] - cache_friendly_times_unroll[k]) / cache_friendly_times[k] for k in cache_friendly_times}
# do element-wise: (no_cache_times - no_cache_times_unroll) / no_cache_times
unroll_speedup = {k: (no_cache_times[k] - no_cache_times_unroll[k]) / no_cache_times[k] for k in no_cache_times}

# truncate them to 2 decimal places
unroll_speedup_cache_friendly = {k: round(v, 2) for k, v in unroll_speedup_cache_friendly.items()}
unroll_speedup = {k: round(v, 2) for k, v in unroll_speedup.items()}

# plot them
plt.figure(figsize=(10, 6))
plt.plot(unroll_speedup_cache_friendly.keys(), unroll_speedup_cache_friendly.values(), 'o-', label='Unroll Speedup with Cache-Friendly Computation', markersize=4)
plt.plot(unroll_speedup.keys(), unroll_speedup.values(), 'o-', label='Unroll Speedup with No-Cache Computation', markersize=4)
plt.xlabel('Matrix Size N')
plt.ylabel('Speedup')
plt.title('Unroll Speedup vs. Matrix Size')
plt.legend()
plt.grid(True)
plt.savefig('unroll_speedup_vs_matrix_size.png')
plt.close()
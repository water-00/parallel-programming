import csv
import subprocess
import re
import matplotlib.pyplot as plt

save_csv = True
save_fig = True

# 存储数据
n_values = []
normal_avg_times = []
unroll_2_avg_times = []
unroll_4_avg_times = []
unroll_8_avg_times = []
unroll_16_avg_times = []
unroll_32_avg_times = []
recursion_avg_times = []

normal_speedup = []
unroll_2_speedup = []
unroll_4_speedup = []
unroll_8_speedup = []
unroll_16_speedup = []
unroll_32_speedup = []
recursion_speedup = []

unroll_22_speedup = []
unroll_42_speedup = []
unroll_82_speedup = []
unroll_162_speedup = []
unroll_322_speedup = []


min_N = 10
max_N = 28
step = 1

# 运行命令并捕获输出
for N in range(min_N, max_N + 1, step):
    print(f"Running: make n_sum N={N} for 3 iterations")

    # 临时存储每次运行的时间
    temp_normal_times = []
    temp_unroll_2_times = []
    temp_unroll_4_times = []
    temp_unroll_8_times = []
    temp_unroll_16_times = []
    temp_unroll_32_times = []
    temp_recursion_times = []
    
    result = subprocess.run(
        f"make n_sum N={N}", shell=True, capture_output=True, text=True)

    for _ in range(3):  # 对每个N运行三次
        result = subprocess.run(
            f"./n_sum_{N}", shell=True, capture_output=True, text=True)

        output = result.stdout

        # 解析输出以提取时间
        normal_match = re.search("Time taken for normal computation: (\d+) microseconds", output)
        unroll_2_match = re.search("Time taken for unroll_2 computation: (\d+) microseconds", output)
        unroll_4_match = re.search("Time taken for unroll_4 computation: (\d+) microseconds", output)
        unroll_8_match = re.search("Time taken for unroll_8 computation: (\d+) microseconds", output)
        unroll_16_match = re.search("Time taken for unroll_16 computation: (\d+) microseconds", output)
        unroll_32_match = re.search("Time taken for unroll_32 computation: (\d+) microseconds", output)
        recursion_match = re.search("Time taken for recursion computation: (\d+) microseconds", output)

        if normal_match and unroll_2_match and unroll_4_match and unroll_8_match and unroll_16_match and unroll_32_match and recursion_match:
            temp_normal_times.append(int(normal_match.group(1)))
            temp_unroll_2_times.append(int(unroll_2_match.group(1)))
            temp_unroll_4_times.append(int(unroll_4_match.group(1)))
            temp_unroll_8_times.append(int(unroll_8_match.group(1)))
            temp_unroll_16_times.append(int(unroll_16_match.group(1)))
            temp_unroll_32_times.append(int(unroll_32_match.group(1)))
            temp_recursion_times.append(int(recursion_match.group(1)))
    
    # 计算平均时间并存储
    if temp_normal_times and temp_unroll_2_times and temp_unroll_4_times and temp_unroll_8_times and temp_unroll_16_times and temp_unroll_32_times and temp_recursion_times:
        avg_normal_time = sum(temp_normal_times) / len(temp_normal_times)
        avg_unroll_2_time = sum(temp_unroll_2_times) / len(temp_unroll_2_times)
        avg_unroll_4_time = sum(temp_unroll_4_times) / len(temp_unroll_4_times)
        avg_unroll_8_time = sum(temp_unroll_8_times) / len(temp_unroll_8_times)
        avg_unroll_16_time = sum(temp_unroll_16_times) / len(temp_unroll_16_times)
        avg_unroll_32_time = sum(temp_unroll_32_times) / len(temp_unroll_32_times)
        avg_recursion_time = sum(temp_recursion_times) / len(temp_recursion_times)

        n_values.append(N)
        normal_avg_times.append(avg_normal_time)
        unroll_2_avg_times.append(avg_unroll_2_time)
        unroll_4_avg_times.append(avg_unroll_4_time)
        unroll_8_avg_times.append(avg_unroll_8_time)
        unroll_16_avg_times.append(avg_unroll_16_time)
        unroll_32_avg_times.append(avg_unroll_32_time)
        recursion_avg_times.append(avg_recursion_time)
        
        if avg_normal_time > 0:
            # speedup compared with normal
            normal_speedup.append(0.0)
            unroll_2_speedup.append((avg_normal_time - avg_unroll_2_time) / avg_normal_time)
            unroll_4_speedup.append((avg_normal_time - avg_unroll_4_time) / avg_normal_time)
            unroll_8_speedup.append((avg_normal_time - avg_unroll_8_time) / avg_normal_time)
            unroll_16_speedup.append((avg_normal_time - avg_unroll_16_time) / avg_normal_time)
            unroll_32_speedup.append((avg_normal_time - avg_unroll_32_time) / avg_normal_time)
            recursion_speedup.append((avg_normal_time - avg_recursion_time) / avg_normal_time)
        else:
            normal_speedup.append(0.0)
            unroll_2_speedup.append(0.0)
            unroll_4_speedup.append(0.0)
            unroll_8_speedup.append(0.0)
            unroll_16_speedup.append(0.0)
            unroll_32_speedup.append(0.0)
            recursion_speedup.append(0.0)
            
        if avg_unroll_2_time > 0:
            # speedup compared with unroll_2
            unroll_22_speedup.append(0.0)
            unroll_42_speedup.append((avg_unroll_2_time - avg_unroll_4_time) / avg_unroll_2_time)
            unroll_82_speedup.append((avg_unroll_2_time - avg_unroll_8_time) / avg_unroll_2_time)
            unroll_162_speedup.append((avg_unroll_2_time - avg_unroll_16_time) / avg_unroll_2_time)
            unroll_322_speedup.append((avg_unroll_2_time - avg_unroll_32_time) / avg_unroll_2_time)
        else:
            unroll_22_speedup.append(0.0)
            unroll_42_speedup.append(0.0)
            unroll_82_speedup.append(0.0)
            unroll_162_speedup.append(0.0)
            unroll_322_speedup.append(0.0)

# 绘制图表
plt.figure(figsize=(10, 6))
plt.plot(n_values, normal_speedup, 'o-', label='Normal Speedup', markersize=4)
plt.plot(n_values, unroll_2_speedup, 'o-', label='Unroll_2 Speedup', markersize=4)
plt.plot(n_values, unroll_4_speedup, 'o-', label='Unroll_4 Speedup', markersize=4)
plt.plot(n_values, unroll_8_speedup, 'o-', label='Unroll_8 Speedup', markersize=4)
plt.plot(n_values, unroll_16_speedup, 'o-', label='Unroll_16 Speedup', markersize=4)
plt.plot(n_values, unroll_32_speedup, 'o-', label='Unroll_32 Speedup', markersize=4)
plt.plot(n_values, recursion_speedup, 'o-', label='Recursion Speedup', markersize=4)

plt.xlabel('Array Size 2^N')
plt.ylabel('Speedup')
# make xlabel all intergers and from min_N to max_N step by 3
plt.xticks(range(min_N, max_N + 1, 3), [str(i) for i in range(min_N, max_N + 1, 3)])

plt.title(f'Speedup Compared with normal vs. Array Size')
plt.legend()
plt.grid(True)
if save_fig:
    plt.savefig('speedup_compared_by_normal_vs_array_size.png')
plt.close()


# draw the speedup of unroll_2

# draw the speedup of unroll_2
plt.figure(figsize=(10, 6))
plt.plot(n_values, unroll_22_speedup, 'o-', label='Unroll_2 Speedup', markersize=4)
plt.plot(n_values, unroll_42_speedup, 'o-', label='Unroll_4 Speedup', markersize=4)
plt.plot(n_values, unroll_82_speedup, 'o-', label='Unroll_8 Speedup', markersize=4)
plt.plot(n_values, unroll_162_speedup, 'o-', label='Unroll_16 Speedup', markersize=4)
plt.plot(n_values, unroll_322_speedup, 'o-', label='Unroll_32 Speedup', markersize=4)

plt.xlabel('Array Size 2^N')
plt.ylabel('Speedup')
# make xlabel all intergers and from min_N to max_N step by 3
plt.xticks(range(min_N, max_N + 1, 3), [str(i) for i in range(min_N, max_N + 1, 3)])

plt.title(f'Speedup Compared with unroll_2 vs. Array Size')
plt.legend()
plt.grid(True)

if save_fig:
    plt.savefig('speedup_compared_by_unroll_2_vs_array_size.png')
plt.close()


# save no_cache_avg_times and cache_friendly_avg_times to a csv file
if save_csv:
    with open(f'result.csv', 'w', newline='') as csvfile:
        # if csv exsits, clean it
        csvfile.truncate()
        writer = csv.writer(csvfile)
        writer.writerow(['N', 'normal', 'unroll_2', 'unroll_4', 'unroll_8', 'unroll_16', 'unroll_32', 'recursion'])
        for n, normal, unroll_2, unroll_4, unroll_8, unroll_16, unroll_32, recursion in zip(n_values, normal_avg_times, unroll_2_avg_times, unroll_4_avg_times, unroll_8_avg_times, unroll_16_avg_times, unroll_32_avg_times, recursion_avg_times):
            writer.writerow([n, normal, unroll_2, unroll_4, unroll_8, unroll_16, unroll_32, recursion])

# 清理生成的文件
subprocess.run("make clean", shell=True)

# 计算有无unroll的加速比
# with open('result_matrix.csv', 'r') as csvfile:
# reader = csv.reader(csvfile)
# next(reader)  # skip header
# results = list(reader)

# no_cache_times = {}
# cache_friendly_times = {}
# for row in results:
#     N = int(row[0])
#     no_cache_times[N] = float(row[1])
#     cache_friendly_times[N] = float(row[2])


# with open('result_matrix_unroll.csv', 'r') as csvfile:
# reader = csv.reader(csvfile)
# next(reader)  # skip header
# results = list(reader)

# no_cache_times_unroll = {}
# cache_friendly_times_unroll = {}
# for row in results:
#     N = int(row[0])
#     no_cache_times_unroll[N] = float(row[1])
#     cache_friendly_times_unroll[N] = float(row[2])

# # do element-wise: (cache_friendly_times - cache_friendly_times_unroll) / cache_friendly_times
# unroll_speedup_cache_friendly = {k: (
# cache_friendly_times[k] - cache_friendly_times_unroll[k]) / cache_friendly_times[k] for k in cache_friendly_times}
# # do element-wise: (no_cache_times - no_cache_times_unroll) / no_cache_times
# unroll_speedup = {k: (
# no_cache_times[k] - no_cache_times_unroll[k]) / no_cache_times[k] for k in no_cache_times}

# # truncate them to 2 decimal places
# unroll_speedup_cache_friendly = {
# k: round(v, 2) for k, v in unroll_speedup_cache_friendly.items()}
# unroll_speedup = {k: round(v, 2) for k, v in unroll_speedup.items()}

# # plot them
# plt.figure(figsize=(10, 6))
# plt.plot(unroll_speedup_cache_friendly.keys(), unroll_speedup_cache_friendly.values(
# ), 'o-', label='Unroll Speedup with Cache-Friendly Computation', markersize=4)
# plt.plot(unroll_speedup.keys(), unroll_speedup.values(), 'o-',
#         label='Unroll Speedup with No-Cache Computation', markersize=4)
# plt.xlabel('Matrix Size N')
# plt.ylabel('Speedup')
# plt.title('Unroll Speedup vs. Matrix Size')
# plt.legend()
# plt.grid(True)
# plt.savefig('unroll_speedup_vs_matrix_size.png')
# plt.close()

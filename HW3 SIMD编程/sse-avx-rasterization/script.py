import subprocess
import numpy as np
import matplotlib.pyplot as plt


def run_rasterization(unit_length):
    times = {'Cache-friendly Mode': [],
             'Cache-unfriendly Mode': [], 'SSE Mode': [], 'AVX Mode': []}
    for _ in range(5):
        command = f'make UL={unit_length}'
        result = subprocess.run(
            command, capture_output=True, text=True, shell=True)
        output = result.stdout.split('\n')
        for line in output:
            if 'took' in line:
                parts = line.split()
                times[parts[0] + ' ' + parts[1]].append(float(parts[3]))
    avg_times = {k: np.mean(v) for k, v in times.items()}
    return avg_times


unit_lengths = range(50, 1051, 50)
results = {'Cache-friendly Mode': [],
           'Cache-unfriendly Mode': [], 'SSE Mode': [], 'AVX Mode': []}

for ul in unit_lengths:
    avg_times = run_rasterization(ul)
    for mode, times in results.items():
        times.append(avg_times[mode])

# Calculate speedup ratios
speedup_ratios = {mode: []
                  for mode in results }

for mode in speedup_ratios:
    for i, time in enumerate(results[mode]):
        if results['Cache-friendly Mode'][i] != 0:
            ratio = results['Cache-friendly Mode'][i] / time
            speedup_ratios[mode].append(ratio)
        else:
            # Handle division by zero if it ever occurs
            speedup_ratios[mode].append(None)

# Plotting the results
plt.figure(figsize=(10, 6))
plt.plot(unit_lengths, speedup_ratios['Cache-unfriendly Mode'],
         label='Cache-unfriendly Mode', marker='o', fillstyle='none', linestyle='-', linewidth=2)

plt.plot(unit_lengths, speedup_ratios['Cache-friendly Mode'], label='Cache-friendly Mode',
         marker='o', fillstyle='none', linestyle='-', linewidth=2)
plt.title('Speedup Relative to Cache-friendly Mode')
plt.xlabel('Unit Length')
plt.ylabel('Speedup Ratio')
plt.legend()
plt.grid(True)
plt.xticks(np.linspace(min(unit_lengths), max(unit_lengths), 11))


plt.figure(figsize=(10, 6))
for mode, ratios in speedup_ratios.items():
    if mode != 'Cache-unfriendly Mode':
        plt.plot(unit_lengths, ratios, label=mode, marker='o',fillstyle='none', linestyle='-', linewidth=2)
plt.title('Speedup Relative to Cache-friendly Mode')
plt.xlabel('Unit Length')
plt.ylabel('Speedup Ratio')
plt.legend()
plt.grid(True)
plt.xticks(np.linspace(min(unit_lengths), max(unit_lengths), 11))
plt.show()

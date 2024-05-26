import pandas as pd
import matplotlib.pyplot as plt

# df = pd.read_csv('dynamic_result.csv')
# df = pd.read_csv('static_result.csv')
df = pd.read_csv('result.csv')

# Plotting
fig, ax = plt.subplots(figsize=(10, 6))
for theta in df['theta'].unique():
    subset = df[df['theta'] == theta]
    ax.plot(subset['n'], subset['time'], marker='o', label=f'Theta={theta}')

ax.set_xlabel('n')
ax.set_ylabel('time')
ax.set_title('Time(ms) vs n for Different Theta Values')
ax.legend()
plt.grid(True)


# Creating a DataFrame to calculate the ratio of times for theta=5 and theta=3 to theta=7
df_ratio = pd.DataFrame({
    "n": [500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000]
})

# Extracting times for each theta
time_theta7 = df[df['theta'] == 7]['time'].values
time_theta5 = df[df['theta'] == 5]['time'].values
time_theta3 = df[df['theta'] == 3]['time'].values

# Calculating ratios
df_ratio['ratio_theta7'] = time_theta7 / time_theta7
df_ratio['ratio_theta5'] = time_theta5 / time_theta7
df_ratio['ratio_theta3'] = time_theta3 / time_theta7

# Plotting the ratios
fig, ax = plt.subplots(figsize=(10, 6))
ax.plot(df_ratio['n'], df_ratio['ratio_theta3'],
        marker='o', label='Theta 3 / Theta 7')
ax.plot(df_ratio['n'], df_ratio['ratio_theta5'],
        marker='o', label='Theta 5 / Theta 7')
ax.plot(df_ratio['n'], df_ratio['ratio_theta7'],
        marker='o', label='Theta 7 / Theta 7')

ax.set_xlabel('n')
ax.set_ylabel('Time Ratio')
ax.set_title('Time Ratio of Theta=5 and Theta=3 to Theta=7')
ax.legend()
plt.grid(True)



plt.show()



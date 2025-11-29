import pandas as pd
import matplotlib.pyplot as plt

density_df = pd.read_csv('results/flow_density_sym.csv')
asym_density_df = pd.read_csv('results/flow_density_asymmetric.csv')
low_aggr = pd.read_csv('results/flow_density_03_aggressive.csv')
med_aggr = pd.read_csv('results/flow_density_06_aggressive.csv')
high_aggr = pd.read_csv('results/flow_density_09_aggressive.csv')

# 0 aggressive drivers, varying density, symmetric rules + lane change
plt.figure(figsize=(14,6))
plt.subplot(1, 2, 1)
plt.plot(density_df['density'], density_df['flow'], 'o-')
plt.xlabel('Density')
plt.ylabel('Average Flow')
plt.title('Flow vs Density (Symmetric Rules)')
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(density_df['density'], density_df['lane_change_rate'], 'o-', color='orange')
plt.xlabel('Density')
plt.ylabel('Lane Change Rate (changes/step)')
plt.title('Lane Change Rate vs Density (Symmetric Rules)')
plt.ylim(0, 0.005)
plt.grid(True)

plt.tight_layout()
plt.savefig('results/repro-symmetric.png')
plt.close()


# 0 aggressive drivers, varying density, asymmetric rules
plt.figure(figsize=(14, 6))
plt.subplot(1, 2, 1)
plt.plot(asym_density_df['density'], asym_density_df['flow'], 'bo-', label='Total Flow')
plt.plot(asym_density_df['density'], asym_density_df['left_flow'], 'cs-', label='Left Lane Flow')
plt.plot(asym_density_df['density'], asym_density_df['right_flow'], 'g^-', label='Right Lane Flow')
plt.xlabel('Density')
plt.ylabel('Average Flow')
plt.title('Flow vs Density (Asymmetric Rules)')
plt.legend()
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(asym_density_df['density'], asym_density_df['lane_change_rate'], 'o-', color='orange')
plt.xlabel('Density')
plt.ylabel('Lane Change Rate')
plt.title('Lane Change Rate vs Density (Asymmetric Rules)')
plt.grid(True)

plt.tight_layout()
plt.savefig('results/repro-asym.png')
plt.close()


# Flow vs density and different aggresiv ratios
plt.figure(figsize=(7, 5))
plt.plot(density_df['density'], density_df['flow'], label='0% Aggressive Drivers')
plt.plot(low_aggr['density'], low_aggr['flow'], label='30% Aggressive Drivers')
plt.plot(med_aggr['density'], med_aggr['flow'], label='60% Aggressive Drivers')
plt.plot(high_aggr['density'], high_aggr['flow'], label='90% Aggressive Drivers')
plt.xlabel('Density')
plt.ylabel('Flow')
plt.title('Flow vs Density and Aggressive Drivers')
plt.grid(True)
plt.ylim(0.20, 0.40)
plt.legend()
plt.savefig('results/density_aggressive.png')
plt.close()